# TestHostingService

## About
UnrealEngineでの自動テストのテストデータをホスティングすることを想定したホスティングサービスとUnrealEngineプラグインです。

- テストケースデータをホスティングするWEBサービス
- 上記WEBサービスのAPIをUnrealEngineから使用するためのプラグイン

で構成されます。

## 動機
移動する、ギミックを起動するといったコマンドで構成されるスクリプト形式の自動テストのテストケースデータをUnrealEngineの外側に持たせるために作成しました。  
外側に持たせることによって以下が可能となります。

- パッケージを作り直すことなくデータ更新が行える
- ゲーム中に記録した情報をテストケースデータとして外部に保存でき、保存されたデータをそのまま使用できる

スクリプト形式のようなデータドリブンな自動テストを作成する場合、テストケースのデータをどこかに持たせる必要があります。  
UnrealEngineのアセットとしてデータを持たせることも一つの方法です。例えば、データテーブルにスクリプトのコマンドを記述する方法があります。  
しかし、UnrealEngineのアセットとして持たせると前述のような事が行えません。  
そのためテストのデータ更新のワークフローが複雑になったりイテレーション速度が落ちる問題が生まれます。  
この問題への解決を目的として作成しました。

## ホスティングサービス
### 起動方法
[service/infra](service/infra)フォルダに移動し `docker-compose up` で起動します。
### 設定方法
ポート番号の指定などは[service/infra/docker-compose.yml](service/infra/docker-compose.yml)で設定することができます。

### TODO:使用方法

### TODO:API
## UnrealEngineプラグイン
[Unreal](Unreal)フォルダにUnrealEngineプロジェクトがあります。  
[Unreal/Plugins/TestHostingService](Unreal/Plugins/TestHostingService)にAPIプラグインがあります。  
[Unreal/Source/TestHostingSampleModule](Unreal/Source/TestHostingSampleModule)にプラグインを利用するサンプルモジュールがあります。  

### TODO:サンプル内容