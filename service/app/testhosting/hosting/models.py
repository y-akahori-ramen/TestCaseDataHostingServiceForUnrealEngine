from django.db import models


class TestCase(models.Model):
    """テストケースモデル

    Args:
        title_path (models.CharField): テストケースパス名
        summary (models.CharField): テストケースの概要
        testcase_data (models.CharField): テストケースのテキストデータ
    """
    title_path = models.CharField(max_length=50)
    summary = models.TextField()
    testcase_data = models.TextField()

    def __str__(self):
        return f'{self.title_path}'
