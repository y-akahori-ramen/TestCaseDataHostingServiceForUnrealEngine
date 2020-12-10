import re
from dataclasses import dataclass
from typing import List
from ..models import TestCase


@dataclass(frozen=True)
class TestCaseListItem:
    """テストケースリストアイテム

    GetTestCaseListItemsの戻り値の型としてしようする。
    テストケース一覧View向けに必要な情報を保持している。

    Attributes:
        name (str): テストケース名
        path_for_url_param (str): テストケースへアクセスする際のURLパラメータ名
        is_directory (bool): ディレクトリ扱いのアイテムか
        summary (str): 一覧リストに表示する用のテストケースの概要説明
    """
    name: str
    path_for_url_param: str
    is_directory: bool
    summary: str


def GetTestCaseListItems(cur_dir_path: str) -> List[TestCaseListItem]:
    """指定したパスに含まれるテストケース一覧取得

    Args:
        cur_dir_path (str): 一覧を取得したいパス

    Returns:
        List[TestCaseListItem]: 指定されたパスに含まれるテストケース一覧。パスが不正な場合は空配列を返す。
    """
    testcases = TestCase.objects.filter(title_path__startswith=f'{cur_dir_path}/')

    # テストケース名を抽出するための正規表現
    testcase_data_match_pattern = re.compile(r'^([^\/]+)$')

    # ディレクトリ名を抽出するための正規表現
    testcase_directory_match_pattern = re.compile(r'^([^\/]+)/')

    # 指定されたパスの文字数　指定されたパス以降の文字列を分離するために使用する
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

            item = TestCaseListItem(
                listitem_name,
                f'{cur_dir_path}/{listitem_name}',
                is_directory,
                list_item_summary
            )
            results.append(item)

    return results


@dataclass(frozen=True)
class BreadcrumbItem:
    """View向けのパンくずリスト情報

    Attributes:
        name (str): 表示名
        url_param (str): 移動する際のURLパラメータ名
        is_active (bool): 現在表示中の位置かどうか
    """
    name: str
    url_param: str
    is_active: bool


def GetBreadcrumbItems(cur_dir_path: str) -> List[BreadcrumbItem]:
    """指定されたパスのパンくずリスト情報を取得する

    Args:
        cur_dir_path (str): パス

    Returns:
        List[BreadcrumbItem]: パンくずリスト情報
    """
    # パスは/区切りとしている
    items = cur_dir_path.split('/')
    result = []
    url_param = ''
    num_item = len(items)
    index = 0
    for item in items:
        index = index + 1
        if item == '':
            # 一番上の階層はRootという名前にする
            result.append(BreadcrumbItem('Root', '', False))
        else:
            url_param += f'/{item}'
            is_active = num_item == index
            result.append(BreadcrumbItem(item, url_param, is_active))

    return result
