import re
from dataclasses import dataclass
from typing import List

from django.shortcuts import render
from django.http import HttpResponse
from django.shortcuts import render

from .models import TestCase


@dataclass(frozen=True)
class TestCaseListItem:
    name: str
    path_for_url_param: str
    is_directory: bool


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
            results.append(TestCaseListItem(listitem_name, f'{cur_dir_path}/{listitem_name}', is_directory))

    return results

# Create your views here.


def index(request):
    if 'path' in request.GET:
        cur_dir = request.GET['path']
    else:
        cur_dir = ''

    list_items = GetTestCaseListItems(cur_dir)

    if len(list_items) == 0:
        return HttpResponse('テストケースが見つかりません')
    else:
        iteminfos = [f'name:{item.name} urlpatm:{item.path_for_url_param} dir:{item.is_directory}' for item in list_items]
    return HttpResponse('<br>'.join(iteminfos))


def listTest(request):
    return render(request, 'hosting/listpage.html')
