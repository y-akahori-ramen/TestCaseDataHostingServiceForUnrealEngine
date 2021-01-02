// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"

namespace TestHosting
{
	/**
	 * @brief APIを使用するのに必要な各種情報
	 */
	class TESTHOSTINGSERVICE_API FContext final
	{
	public:
		/**
		 * @brief コンストラクタ
		 * @param User ユーザー名
		 * @param Password パスワード
		 * @param ServiceURI テストホスティングサービスのURI
		 */
		FContext(const FString& User, const FString& Password, const FString& ServiceURI);

		/**
		 * @brief テストケース取得用のURIを取得する
		 * @param TestCaseDataName 取得したいテストケース名
		 */
		FString GetURIGetTestCaseData(FString TestCaseDataName) const;

		/**
		 * @brief テストケース追加用のURIを取得する
		 */
		FString GetURIAddTestCaseData() const;

		/**
		 * @brief テストケース名一覧取得用のURIを取得する
		 */
		FString GetURIGetTestCaseNameList() const;

		/**
		 * @brief HTTPリクエストにBasic認証ヘッダーを追加する
		 * @param Request HTTPリクエスト
		 */
		void AppendBasicAuthHeader(TSharedRef<IHttpRequest> Request) const;

	private:
		const FString UserPasswordForBasicAuth;
		const FString ServiceURI;
		const FString AddURI;
		const FString GetListURI;
	};

	/**
	 * @brief リクエスト結果
	 */
	class TESTHOSTINGSERVICE_API FRequestResult final
	{
	public:
		FRequestResult(bool bIsSuccess, const FString& Message)
			: bIsSuccess(bIsSuccess), Message(Message)
		{
		}

		FRequestResult()
			: bIsSuccess(false), Message(FString())
		{
		}

		/**
		 * @brief 成功したか
		 */
		bool IsSuccess() const { return bIsSuccess; }

		/**
		 * @brief リクエスト詳細メッセージ
		 *		　 リクエストに失敗した場合のエラーの詳細等が入ります。
		 */
		const FString& GetMessage() const { return Message; }
	private:
		bool bIsSuccess;
		FString Message;
	};

	/**
	 * @brief テストケースデータ
	 */
	class TESTHOSTINGSERVICE_API FTestCaseData final
	{
	public:
		FTestCaseData() = default;

		/**
		 * @brief コンストラクタ
		 * @param Name テストケース名
		 * @param Commands テストコマンド配列
		 * @param Summary テストケースサマリ情報
		 */
		FTestCaseData(const FString& Name, const TArray<FString>& Commands, const FString& Summary);

		const FString& GetName() const { return Name; }
		const TArray<FString>& GetCommands() const { return Commands; }
		const FString& GetSummary() const { return Summary; }

	private:
		FString Name;
		TArray<FString> Commands;
		FString Summary;
	};

	/**
	 * @brief テストケースデータ取得リクエスト結果型
	 */
	using FGetTestCaseDataResult = TTuple<FRequestResult, FTestCaseData>;

	/**
	 * @brief テストケースデータ取得リクエスト　非同期版
	 * @param TestCaseName 取得したいテストケースデータの名前
	 * @param Context 認証情報などのコンテキスト
	 * @return 結果が入るTFuture
	 */
	TESTHOSTINGSERVICE_API TFuture<FGetTestCaseDataResult> RequestGetTestCaseDataAsync(const FString& TestCaseName, const FContext& Context);

	/**
	 * @brief テストケースデータ取得リクエスト　同期版
	 * @param TestCaseName 取得したいテストケースデータの名前
	 * @param Context 認証情報などのコンテキスト
	 * @return 結果
	 */
	TESTHOSTINGSERVICE_API FGetTestCaseDataResult RequestGetTestCaseData(const FString& TestCaseName, const FContext& Context);


	/**
	 * @brief テストケースデータ追加・更新リクエスト 非同期版
	 *		   既存のテストケースデータに対して送ると更新扱いとなります
	 * @param TestCaseData 対象テストケースデータ
	 * @param Context 認証情報などのコンテキスト
	 * @return 結果が入るTFuture
	 */
	TESTHOSTINGSERVICE_API TFuture<FRequestResult> RequestAddTestCaseDataAsync(const FTestCaseData& TestCaseData, const FContext& Context);

	/**
	 * @brief テストケースデータ追加・更新リクエスト 同期版
	 *		   既存のテストケースデータに対して送ると更新扱いとなります
	 * @param TestCaseData 対象テストケースデータ
	 * @param Context 認証情報などのコンテキスト
	 * @return 結果
	 */
	TESTHOSTINGSERVICE_API FRequestResult RequestAddTestCaseData(const FTestCaseData& TestCaseData, const FContext& Context);

	/**
	 * @brief テストケースデータ名一覧取得リクエスト結果型
	 */
	using FGetTestCaseListResult = TTuple<FRequestResult, TArray<FString>>;

	/**
	 * @brief テストケースデータ名一覧取得リクエスト　非同期版
	 * @param Context 認証情報などのコンテキスト
	 * @return リクエスト結果が入るTFuture
	 */
	TESTHOSTINGSERVICE_API TFuture<FGetTestCaseListResult> RequestGetTestCaseListAsync(const FContext& Context);

	/**
	 * @brief テストケースデータ名一覧取得リクエスト　同期版
	 * @param Context 認証情報などのコンテキスト
	 * @return リクエスト結果
	 */
	TESTHOSTINGSERVICE_API FGetTestCaseListResult RequestGetTestCaseList(const FContext& Context);
};
