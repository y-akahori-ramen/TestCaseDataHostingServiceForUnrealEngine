import re
from dataclasses import dataclass
from typing import List

from django.http import HttpResponse, HttpResponseRedirect, HttpResponseNotFound
from django.shortcuts import render, get_object_or_404
from django.urls import reverse
from django.http.response import JsonResponse
from django.contrib.auth.decorators import login_required
from django.contrib.auth import authenticate, login

from rest_framework.decorators import api_view, permission_classes
from rest_framework.response import Response
from rest_framework import permissions
from .models import TestCase


@dataclass(frozen=True)
class TestCaseListItem:
    name: str
    path_for_url_param: str
    is_directory: bool
    summary: str


@dataclass(frozen=True)
class BreadcrumbItem:
    name: str
    url_param: str
    is_active: bool


def GetTestCaseListItems(cur_dir_path) -> List[TestCaseListItem]:
    testcases = TestCase.objects.filter(title_path__startswith=f'{cur_dir_path}/')

    testcase_data_match_pattern = re.compile(r'^([^\/]+)$')
    testcase_directory_match_pattern = re.compile(r'^([^\/]+)/')
    cur_dir_path_len = len(cur_dir_path)
    results = []

    for testcase in testcases:
        # cur_dir_path以降のパスを取得
        curdir_removed_path = testcase.title_path[cur_dir_path_len+1:]

        # 以降のパスが/を含まず行末まで行っている場合はファイルとして確定
        # 以降のパスに/が含まれる場合は/までの名前がディレクトリとなる
        testcase_data_match = testcase_data_match_pattern.match(curdir_removed_path)
        is_directory = False
        if testcase_data_match is not None:
            is_directory = False
            listitem_name = testcase_data_match.group(1)
        else:
            testcase_directory_match = testcase_directory_match_pattern.match(curdir_removed_path)
            is_directory = True
            listitem_name = testcase_directory_match.group(1)

        # listitem_nameが結果のリストに存在しなければ追加
        # /Walk/Hoge/1, /Walk/Hoge/2のようなテストケースがあり、/Walkをcur_dir_pathにすると
        # listitem_nameがHogeのものが２つでる。これはディレクトリ扱いとなる。
        # リスト表示画面では１アイテムだけ表示すれば良いので１つだけ登録するようにする。
        exist = next((item for item in results if item.name == listitem_name), None)
        if exist is None:
            list_item_summary = ""
            if is_directory:
                list_item_summary = " \n "
            else:
                # 概要が複数行の場合、先頭１行を表示させる
                if len(testcase.summary) > 0:
                    list_item_summary = testcase.summary.splitlines()[0]

                list_item_summary += f'\nテストパス名:{cur_dir_path}/{listitem_name}'

            results.append(TestCaseListItem(
                listitem_name, f'{cur_dir_path}/{listitem_name}', is_directory, list_item_summary))

    return results


def GetBreadcrumbItems(cur_dir_path) -> List[BreadcrumbItem]:
    items = cur_dir_path.split('/')
    result = []
    url_param = ''
    num_item = len(items)
    index = 0
    for item in items:
        index = index + 1
        if item == '':
            result.append(BreadcrumbItem('Root', '', False))
        else:
            url_param += f'/{item}'
            result.append(BreadcrumbItem(item, url_param, num_item == index))

    return result

def signin(request):
    if request.method == 'GET':
        if request.user.is_authenticated:
            return HttpResponseRedirect(reverse('hosting:index'))
        else:
            return render(request, 'hosting/signin.html')
    elif request.method == 'POST':
        username = request.POST['inputUser']
        userpassword = request.POST['inputPassword']
        user = authenticate(request,username=username, password=userpassword)
    
        if user is not None:
            login(request,user=user)
            # ログインしていない状態でページにアクセスした場合このビューが表示され、その場合はnextに戻り先が入っている
            # nextが存在すればそちらへ移動し、nextがなければトップページに移動
            if 'next' in request.GET:
                return HttpResponseRedirect(request.GET['next'])
            else:
                return HttpResponseRedirect(reverse('hosting:index'))
        else:
            return render(request, 'hosting/signin.html')
    else:
        return render(request, 'hosting/signin.html')

@login_required(login_url='/signin')
def index(request):
    if 'path' in request.GET:
        cur_dir = request.GET['path']
    else:
        cur_dir = ''

    list_items = GetTestCaseListItems(cur_dir)
    breadcrumb_items = GetBreadcrumbItems(cur_dir)
    params = {
        'list_items': list_items,
        'breadcrumb_items': breadcrumb_items,
        'create_initial_testcasename': cur_dir + '/',
    }

    return render(request, 'hosting/listpage.html', params)

@login_required(login_url='/signin')
def edit_get(request):
    if 'path' in request.GET:
        path = request.GET['path']
    else:
        path = ''

    # submitされた場合にデータの更新を行い、以前のリストページへリダイレクト

    testcase_data = get_object_or_404(TestCase, title_path=path)

    breadcrumb_items = GetBreadcrumbItems(path)

    # パンくずリストの末尾が自分のデータのパスなのでそれより１つ前が戻るべきパスとなる
    backurl_path = breadcrumb_items[-2].url_param

    params = {
        'breadcrumb_items': breadcrumb_items,
        'summary': testcase_data.summary,
        'data': testcase_data.testcase_data,
        'backurl_path': backurl_path,
        'cur_path': path,
    }
    return render(request, 'hosting/editpage.html', params)


def edit_post(request):
    testcase_data = get_object_or_404(TestCase, title_path=request.GET['path'])
    testcase_data.summary = request.POST['summary']
    testcase_data.testcase_data = request.POST['testcasedata']
    testcase_data.save()

    # 一つ前のリストに戻す
    breadcrumb_items = GetBreadcrumbItems(request.GET['path'])
    backurl_path = breadcrumb_items[-2].url_param

    redirect_uri = reverse('hosting:index') + f'?path={backurl_path}'
    return HttpResponseRedirect(redirect_uri)


@login_required(login_url='/signin')
def edit(request):
    if request.method == 'GET':
        return edit_get(request)
    elif request.method == 'POST':
        return edit_post(request)


@dataclass(frozen=True)
class CheckNameResult:
    is_valid: bool
    desc: str


def check_valid_new_name(name: str, ignore_name: str) -> CheckNameResult:
    if len(name) == 0:
        return CheckNameResult(False, '名前が空です')
    # 同じ名前のテストケースが存在する場合は無効
    if ignore_name is None:
        testcases = TestCase.objects
    else:
        testcases = TestCase.objects.exclude(title_path=ignore_name)

    if testcases.filter(title_path__startswith=name).exists():
        return CheckNameResult(False, f'同じ名前のテストケースが存在します\n{name}')
    # 一番先頭が/始まりで末尾は/じゃないこと。/と/の間に文字があること
    if re.match(r'^(/[\w-]+)*$', name) is None:
        return CheckNameResult(False, f'名前のフォーマットが不正です\n{name}')

    # 作ろうとしている名前のパスの一部がテストケースデータそのものを含む場合はエラー
    # 作ろうとしている側はディレクトリとしてその名前が機能するようにしたいが、すでにデータの実体として存在しているためエラー
    words = name.split('/')[1:]
    path = ""
    for word in words:
        path = path + "/" + word
        is_testcase_data = len(testcases.filter(title_path=path)) == 1
        if is_testcase_data:
            return CheckNameResult(False, f'作ろうとしている名前のパスの一部がテストケースデータとして存在します\n{name}\n{path}')
    return CheckNameResult(True, '')


@login_required(login_url='/signin')
def create(request):
    if request.method == 'POST':
        new_name = request.POST['testcaseName']
        check_result = check_valid_new_name(new_name, None)
        if check_result.is_valid:
            new_testcase = TestCase(title_path=new_name)
            new_testcase.save()
            redirect_uri = reverse('hosting:edit') + f'?path={new_name}'
            return HttpResponseRedirect(redirect_uri)
        else:
            params = {
                'error_msg': check_result.desc,
            }
            return render(request, 'hosting/errorpage.html', params)
    else:
        return HttpResponseNotFound()


@login_required(login_url='/signin')
def delete(request):
    if request.method == 'POST':
        delete_name = request.POST['testcaseName']
        testcase_data = get_object_or_404(TestCase, title_path=delete_name)
        testcase_data.delete()
        return HttpResponseRedirect(reverse('hosting:index'))
    else:
        return HttpResponseNotFound()


@login_required(login_url='/signin')
def rename(request):
    if request.method == 'POST':
        new_name = request.POST['testcaseName']
        old_name = request.POST['oldName']
        check_result = check_valid_new_name(new_name, old_name)
        if check_result.is_valid:
            testcase_data = get_object_or_404(TestCase, title_path=old_name)
            testcase_data.title_path = new_name
            testcase_data.save()
            return HttpResponseRedirect(reverse('hosting:index'))
        else:
            params = {
                'error_msg': check_result.desc,
            }
            return render(request, 'hosting/errorpage.html', params)

    else:
        return HttpResponseNotFound()


class TestCaseDataConverter:
    @staticmethod
    def covert(testcase_data: str):
        """テストケース文字列をテストコマンドに変換する

        文字列中のコメントや不用な空白を取り除きコマンド文字列のリストに変換する。
        コマンドフォーマットとして正しくないものがある場合例外を送出します。

        Args:
            testcase_data (str): テストケースデータ文字列

        Returns:
            List(str): コマンド文字列リスト
        """
        commands = []

        for line in testcase_data.splitlines():
            command = TestCaseDataConverter.__convert_command(line)
            if command is not None:
                commands.append(command)

        return commands

    __remove_head_space_pattern = re.compile(r'^\s*(\S.*)$')

    @staticmethod
    def __convert_command(command_str: str):
        comment_str = "//"
        comment_str_idx = command_str.find(comment_str)
        if comment_str_idx >= 0:
            comment_removed_line = command_str[:comment_str_idx]
        else:
            comment_removed_line = command_str

        if len(comment_removed_line) == 0:
            return None
        else:
            # 行頭の空白を取り除く
            match = TestCaseDataConverter.__remove_head_space_pattern.match(comment_removed_line)
            if match is None:
                # 空白のみの行のためスキップ
                return None
            else:
                normalized_command_str = match.group(1)
                return normalized_command_str


def get_testdata_for_json(name: str) -> {}:
    result_info = {
        'result': True,
        'desc': None,
        'name': name,
    }

    query_result = TestCase.objects.filter(title_path=name)

    if not query_result.exists():
        result_info['result'] = False
        result_info['desc'] = 'テストケースが存在していません'
        return {'result': result_info, 'commands': []}

    if len(query_result) > 1:
        result_info['result'] = False
        result_info['desc'] = '同名のテストケースが複数存在します'
        return {'result': result_info, 'commands': []}

    raw_commands = TestCaseDataConverter.covert(query_result[0].testcase_data)
    include_cmd_pattern = re.compile(r'^include\s+-path\s+(\S+)')

    commands = []
    for command in raw_commands:
        if command.startswith('include'):
            include_match = include_cmd_pattern.match(command)
            if include_match is None:
                result_info['result'] = False
                result_info['desc'] = f'includeコマンドフォーマットエラーです Command: {command}'
                return {'result': result_info, 'commands': []}
            else:
                # include指定されているテストケースのコマンドを取得
                include_path = include_match.group(1)
                include_result = get_testdata_for_json(include_path)
                include_result_info = include_result['result']
                if include_result_info['result'] == True:
                    commands += include_result['commands']
                else:
                    include_error_desc = include_result_info['desc']
                    result_info['result'] = False
                    result_info['desc'] = f'include先のテストケースでエラーが発生しました Command: {command}\ninclude先エラー:{include_error_desc}'
                    return {'result': result_info, 'commands': []}
        else:
            commands.append(command)

    result_info['desc'] = '成功しました'
    return {'result': result_info, 'commands': commands}

@api_view(['GET'])
@permission_classes([permissions.IsAuthenticated])
def get_json_data(request):
    if 'path' in request.query_params:
        path = request.query_params['path']
    else:
        path = ''

    responcse = get_testdata_for_json(path)
    print(responcse)
    return Response(responcse)
