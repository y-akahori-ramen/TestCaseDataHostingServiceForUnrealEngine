FROM python:3.8.5

RUN mkdir /home/data
COPY ./data/ /home/data

# 必要なパッケージインストール
RUN pip install --upgrade pip
RUN pip install -r /home/data/requirements.txt

# コンテナ起動時に実行するスクリプトに権限が必要なため設定
RUN chmod 755 /home/data/entrypoint.sh