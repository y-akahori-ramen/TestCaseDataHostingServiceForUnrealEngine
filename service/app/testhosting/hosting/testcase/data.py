import re
from typing import List, Dict
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
