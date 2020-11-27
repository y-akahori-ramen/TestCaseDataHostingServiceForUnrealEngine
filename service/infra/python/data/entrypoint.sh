#!/bin/sh
# 不要になる静的コンテンツを残したくないためclearオプションで毎回コピー前に削除する
python /home/app/testhosting/manage.py collectstatic --noinput --clear

# superuserの作成
python /home/app/testhosting/manage.py createsuperuser --email $SUPER_USER_EMAIL --username $SPUER_USER_NAME --noinput

# マイグレーション
# ※djangoプロジェクト内のアプリが増えた場合はその分 makemigrations アプリ名 が必要
python /home/app/testhosting/manage.py makemigrations
python /home/app/testhosting/manage.py migrate

# サーバー起動
uwsgi --ini /home/data/django_uwsgi.ini