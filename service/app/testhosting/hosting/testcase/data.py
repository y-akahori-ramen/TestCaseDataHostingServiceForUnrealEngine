import re
from typing import List, Dict
from dataclasses import dataclass
from ..models import TestCase


def _convert_test_commands(testcase_data: str) -> List[str]:
    """テストケース文字列をテストコマンドに変換する

    文字列中のコメントや不用な空白を取り除きコマンド文字列のリストに変換する。
    コマンドフォーマットとして正しくないものがある場合例外を送出します。

    Args:
        testcase_data (str): テストケースデータ文字列

    Returns:
        List(str): コマンド文字列リスト
    """
    commands = []

    remove_head_space_pattern = re.compile(r'^\s*(\S.*)$')

    for line in testcase_data.splitlines():
        # コメント部分を取り除く
        comment_str_idx = line.find('//')
        if comment_str_idx >= 0:
            normalized_line = line[:comment_str_idx]
        else:
            normalized_line = line

        # 空白じゃない部分を抽出
        match = remove_head_space_pattern.match(normalized_line)
        if match is None:
            # 空白のみだった場合はコマンドなし
            normalized_line = None
        else:
            normalized_line = match.group(1)

        if normalized_line is not None:
            commands.append(normalized_line)

    return commands


def _get_raw_test_commands(name: str) -> List[str]:
    """テストケースのinclude解決を行っていない状態の生のコマンドを取得する

    Args:
        name (str): テストケース名

    Raises:
        Exception: 取得に失敗した場合例外を送出します

    Returns:
        List[str]: テストコマンド配列
    """
    query_result = TestCase.objects.filter(title_path=name)

    if not query_result.exists():
        raise Exception(f'テストケース {name} が存在していません')

    if len(query_result) > 1:
        raise Exception(f'テストケース {name} が複数存在します')

    return _convert_test_commands(query_result[0].testcase_data)


def _resolve_commands(commands: List[str]) -> List[str]:
    """テストケースコマンドのinclude解決を行い、実際に実行されるコマンドを取得する  

    Args:
        commands (List[str]): コマンド配列

    Raises:
        Exception: 解決に失敗した場合例外を送出します

    Returns:
        List[str]: 解決済みコマンド配列
    """
    resolved_commands = []
    include_cmd_pattern = re.compile(r'^include\s+-path\s+(\S+)')

    for command in commands:
        if command.startswith('include'):
            # includeコマンドの解決を行う
            include_match = include_cmd_pattern.match(command)
            if include_match is None:
                raise Exception(f'includeコマンドフォーマットエラーです Command: {command}')
            else:
                # include指定されているテストケースのコマンドを取得
                include_path = include_match.group(1)

                try:
                    include_commands = _get_raw_test_commands(include_path)
                    include_commands = _resolve_commands(include_commands)
                    resolved_commands += include_commands
                except Exception as exp:
                    raise Exception(f'include先のテストケースでエラーが発生しました Command:{command}\ninclude先エラー:{exp}')
        else:
            resolved_commands.append(command)

    return resolved_commands


def get_testcase_commands(name: str) -> List[str]:
    """テストケースコマンド取得

    Args:
        name (str): テストケース名

    Raises:
        Exception: 解決に失敗した場合例外を送出します

    Returns:
        List[str]: テストケースコマンド
    """
    commands = _get_raw_test_commands(name)
    commands = _resolve_commands(commands)
    return commands


@dataclass(frozen=True)
class ValidateNameResult:
    is_valid: bool
    desc: str


def _validate_name(name: str, ignore_name: str) -> ValidateNameResult:
    """有効な名前かの検証処理

    除外したい名前を指定するとリネームする名前が有効かを検証する関数として使用することができる

    Args:
        name (str): 検証したい名前
        ignore_name (str): 除外したい名前

    Returns:
        ValidateNameResult: 検証結果
    """
    if len(name) == 0:
        return ValidateNameResult(False, '名前が空です')
    # 同じ名前のテストケースが存在する場合は無効
    if ignore_name is None:
        testcases = TestCase.objects
    else:
        testcases = TestCase.objects.exclude(title_path=ignore_name)

    if testcases.filter(title_path__startswith=name).exists():
        return ValidateNameResult(False, f'同じ名前のテストケースが存在します\n{name}')
    # 一番先頭が/始まりで末尾は/じゃないこと。/と/の間に文字があること
    if re.match(r'^(/[\w-]+)*$', name) is None:
        return ValidateNameResult(False, f'名前のフォーマットが不正です\n{name}')

    # 作ろうとしている名前のパスの一部がテストケースデータそのものを含む場合はエラー
    # 作ろうとしている側はディレクトリとしてその名前が機能するようにしたいが、すでにデータの実体として存在しているためエラー
    words = name.split('/')[1:]
    path = ""
    for word in words:
        path = path + "/" + word
        is_testcase_data = len(testcases.filter(title_path=path)) == 1
        if is_testcase_data:
            return ValidateNameResult(False, f'作ろうとしている名前のパスの一部がテストケースデータとして存在します\n{name}\n{path}')
    return ValidateNameResult(True, '')


def validate_new_name(name: str) -> ValidateNameResult:
    """新規作成するテストケース名として有効な名前か検証する

    Args:
        name (str): 新規作成するテストケース名

    Returns:
        ValidateNameResult: 検証結果
    """
    return _validate_name(name, None)


def validate_rename_name(name: str, oldname: str) -> ValidateNameResult:
    """名前変更しようとしているテストケースの変更予定の名前が有効な名前か検証する

    Args:
        name (str): 変更予定の名前
        oldname (str): 現在の名前

    Returns:
        ValidateNameResult: 検証結果
    """
    return _validate_name(name, oldname)
