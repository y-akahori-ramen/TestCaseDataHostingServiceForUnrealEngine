from dataclasses import dataclass
from typing import Optional

from rest_framework.decorators import api_view, permission_classes
from rest_framework.response import Response
from rest_framework import permissions, status

from .models import TestCase

from .testcase.data import get_testcase_commands, get_testcase_summary, validate_new_name

_KEY_NAME = 'Name'
_KEY_SUMMARY = 'Summary'
_KEY_COMMANDS = 'Commands'
_KEY_MESSAGE = 'Message'
_KEY_TESTCASES = 'TestCases'
_MESSAGE_SUCCESS = "成功しました"


@api_view(['GET'])
@permission_classes([permissions.IsAuthenticated])
def get_json_data(request):
    """実際のテストケースデータをJSONで返します。

    返すテストケースデータはinclude解決やコメントアウト処理がされており実際に実行されるデータが返されます
    """
    if 'path' in request.query_params:
        path = request.query_params['path']
    else:
        path = ''

    try:
        commands = get_testcase_commands(path)
        summary = get_testcase_summary(path)
        response = {
            _KEY_NAME: path,
            _KEY_COMMANDS: commands,
            _KEY_SUMMARY: summary,
            _KEY_MESSAGE: _MESSAGE_SUCCESS
        }
        return Response(response)
    except Exception as exp:
        response = {
            _KEY_NAME: path,
            _KEY_COMMANDS: [],
            _KEY_SUMMARY: '',
            _KEY_MESSAGE: f'{exp}'
        }
        return Response(response, status=status.HTTP_400_BAD_REQUEST)


@api_view(['GET'])
@permission_classes([permissions.IsAuthenticated])
def get_list(request):
    """全テストケースの名前をJSONで取得します
    """
    testcase_names = []
    testcases = TestCase.objects.all()
    for testcase in testcases:
        testcase_names.append(testcase.title_path)

    return Response({_KEY_TESTCASES: testcase_names, _KEY_MESSAGE: _MESSAGE_SUCCESS})


@dataclass(frozen=True)
class _TestCaseData:
    """送られてきたテストケースデータを処理しやすく加工したデータ
    """
    name: str
    summary: str
    testcase_data: str

    def apply(self, data: TestCase):
        """テストケースモデルオブジェクトへ自分のデータを適用する

        Args:
            data (TestCase): 適用先のテストケースモデルオブジェクト
        """
        data.testcase_data = self.testcase_data
        data.title_path = self.name
        if self.summary is not None:
            data.summary = self.summary


def _convert_testcase_data(dist) -> Optional[_TestCaseData]:
    """ディクショナリから処理しやすいように_TestCaseDataに変換する

    ディクショナリ例
    {'Commands': ['Command1', 'Command2', 'Command3'], 'Name': '/aaa/a/sample', 'Summary':'SummaryData'}

    Args:
        dist : ディクショナリ

    Returns:
        [_TestCaseData]: テストケースデータ。ディクショナリ内に必要なキーが無い場合はNoneを返します
    """
    if _KEY_NAME in dist:
        name = dist[_KEY_NAME]
    else:
        return None

    if _KEY_COMMANDS in dist:
        commands = dist[_KEY_COMMANDS]
        testcase_data = '\n'.join(commands)
    else:
        return None

    if _KEY_SUMMARY in dist:
        summary = dist[_KEY_SUMMARY]
    else:
        summary = None

    return _TestCaseData(name, summary, testcase_data)


@api_view(['POST'])
@permission_classes([permissions.IsAuthenticated])
def add_testcase(request):
    """テストケースの追加、既存のテストケースへの追加の場合は更新扱いとなります。
    """

    testcase_data = _convert_testcase_data(request.data)
    if testcase_data is None:
        return Response({_KEY_MESSAGE: '必要なデータが不足しています'}, status=status.HTTP_400_BAD_REQUEST)

    is_overwrite = TestCase.objects.filter(title_path=testcase_data.name).exists()

    if is_overwrite:
        testcase = TestCase.objects.get(title_path=testcase_data.name)
    else:
        # 新規追加のテストデータのため名前の正当性チェックを行う
        validate_name_result = validate_new_name(testcase_data.name)
        if not validate_name_result.is_valid:
            return Response({_KEY_MESSAGE: validate_name_result.desc}, status=status.HTTP_400_BAD_REQUEST)
        testcase = TestCase()

    # 送られてきたデータで更新
    testcase_data.apply(testcase)

    testcase.save()
    return Response({_KEY_MESSAGE: _MESSAGE_SUCCESS})
