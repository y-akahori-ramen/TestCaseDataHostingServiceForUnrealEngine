#!/bin/sh
# 不要になる静的コンテンツを残したくないためclearオプションで毎回コピー前に削除する
python /home/app/testhosting/manage.py collectstatic --noinput --clear

# マイグレーション
python /home/app/testhosting/manage.py makemigrations
python /home/app/testhosting/manage.py migrate

# superuserの作成
python /home/app/testhosting/manage.py createsuperuser --email $SUPER_USER_EMAIL --username $SPUER_USER_NAME --noinput

# サーバー起動
uwsgi --ini /home/data/django_uwsgi.ini