import re
from dataclasses import dataclass
from typing import List

from django.http import HttpResponse, HttpResponseRedirect
from django.shortcuts import render, get_object_or_404
from django.urls import reverse

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
            if is_directory:
                list_item_summary = ""
            else:
                # 概要が複数行の場合、先頭１行を表示させる
                list_item_summary = testcase.summary.splitlines()[0]
            results.append(TestCaseListItem(listitem_name, f'{cur_dir_path}/{listitem_name}', is_directory, list_item_summary))

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

def index(request):
    if 'path' in request.GET:
        cur_dir = request.GET['path']
    else:
        cur_dir = ''

    list_items = GetTestCaseListItems(cur_dir)
    breadcrumb_items = GetBreadcrumbItems(cur_dir)
    params = {
        'list_items' : list_items,
        'breadcrumb_items' : breadcrumb_items
    }

    return render(request, 'hosting/listpage.html', params)


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
        'breadcrumb_items' : breadcrumb_items,
        'summary' : testcase_data.summary,
        'data' : testcase_data.testcase_data,
        'backurl_path' : backurl_path,
        'cur_path' : path,
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

def edit(request):
    if request.method == 'GET':
        return edit_get(request)
    elif request.method == 'POST':
        return edit_post(request)



