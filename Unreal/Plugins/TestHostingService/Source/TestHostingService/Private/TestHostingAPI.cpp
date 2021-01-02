// Fill out your copyright notice in the Description page of Project Settings.


#include "TestHostingAPI.h"
#include "Interfaces/IHttpResponse.h"
#include "http.h"
#include "HttpModule.h"
#include "HttpManager.h"
#include "Misc/Base64.h"

namespace TestHosting
{
	namespace Internal
	{
		class FJsonKeys final
		{
		public:
			static const FString KeyCommands;
			static const FString KeyName;
			static const FString KeyMessage;
			static const FString KeySummary;
			static const FString KeyTestCases;
		};

		const FString FJsonKeys::KeyCommands = TEXT("Commands");
		const FString FJsonKeys::KeyName = TEXT("Name");
		const FString FJsonKeys::KeyMessage = TEXT("Message");
		const FString FJsonKeys::KeySummary = TEXT("Summary");
		const FString FJsonKeys::KeyTestCases = TEXT("TestCases");

		static const FRequestResult InvalidResult(false, TEXT("Invalid"));
		static const FTestCaseData InvalidTestCaseData;
		static const FRequestResult ConnectFailedResult(false, TEXT("Connect failed"));
	}


	//---------------------------------------------------------------------
	// APIを使用するのに必要な各種情報
	//---------------------------------------------------------------------
	FContext::FContext(const FString& User, const FString& Password, const FString& ServiceURI)
		: UserPasswordForBasicAuth(FBase64::Encode(FString::Format(TEXT("{0}:{1}"), {*User, *Password}))),
		  ServiceURI(ServiceURI),
		  AddURI(FString::Format(TEXT("{0}/add/"), {*ServiceURI})),
		  GetListURI(FString::Format(TEXT("{0}/list/"), {*ServiceURI}))
	{
	}

	FString FContext::GetURIGetTestCaseData(FString TestCaseDataName) const
	{
		return FString::Format(TEXT("{0}/json/?path={1}"), {*ServiceURI, *TestCaseDataName});
	}

	FString FContext::GetURIAddTestCaseData() const
	{
		return AddURI;
	}

	FString FContext::GetURIGetTestCaseNameList() const
	{
		return GetListURI;
	}

	void FContext::AppendBasicAuthHeader(TSharedRef<IHttpRequest> Request) const
	{
		const FString AuthorizationHeaderValue = FString::Format(TEXT("Basic {0}"), {*UserPasswordForBasicAuth});
		Request->AppendToHeader(TEXT("Authorization"), AuthorizationHeaderValue);
	}

	//---------------------------------------------------------------------
	// テストケースデータ
	//---------------------------------------------------------------------
	FTestCaseData::FTestCaseData(const FString& Name, const TArray<FString>& Commands, const FString& Summary)
		: Name(Name),
		  Commands(Commands),
		  Summary(Summary)
	{
	}

	//---------------------------------------------------------------------
	// テストケースデータ取得リクエスト
	//---------------------------------------------------------------------
	TFuture<FGetTestCaseDataResult> RequestGetTestCaseDataAsync(const FString& TestCaseName, const FContext& Context)
	{
		TSharedPtr<TPromise<FGetTestCaseDataResult>> Promise = MakeShareable(new TPromise<FGetTestCaseDataResult>());
		TFuture<FGetTestCaseDataResult> Future = Promise->GetFuture();


		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

		Request->SetURL(Context.GetURIGetTestCaseData(TestCaseName));
		Context.AppendBasicAuthHeader(Request);

		auto CompleteFunc = [Promise](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			if (!bConnectedSuccessfully)
			{
				Promise->SetValue(FGetTestCaseDataResult(Internal::ConnectFailedResult, Internal::InvalidTestCaseData));
			}
			else
			{
				if (Response->GetResponseCode() == EHttpResponseCodes::Ok)
				{
					FString Message;
					TArray<FString> Commands;
					FString Name;
					FString Summary;

					const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
					TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

					const bool bParseSuccess = FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
						JsonObject->TryGetStringField(Internal::FJsonKeys::KeyName, Name) &&
						JsonObject->TryGetStringField(Internal::FJsonKeys::KeySummary, Summary) &&
						JsonObject->TryGetStringArrayField(Internal::FJsonKeys::KeyCommands, Commands) &&
						JsonObject->TryGetStringField(Internal::FJsonKeys::KeyMessage, Message);

					if (bParseSuccess)
					{
						Promise->SetValue(
							FGetTestCaseDataResult(
								FRequestResult(true, Message),
								FTestCaseData(Name, Commands, Summary)
							)
						);
					}
					else
					{
						const FString Msg = FString::Format(TEXT("ResponseParseFailed. Content: {0}"), {*Response->GetContentAsString()});
						Promise->SetValue(
							FGetTestCaseDataResult(
								FRequestResult(false, Msg),
								Internal::InvalidTestCaseData
							)
						);
					}
				}
				else
				{
					Promise->SetValue(
						FGetTestCaseDataResult(
							FRequestResult(false, Response->GetContentAsString()),
							Internal::InvalidTestCaseData
						)
					);
				}
			}
		};

		Request->OnProcessRequestComplete().BindLambda(CompleteFunc);

		Request->ProcessRequest();


		return Future;
	}

	FGetTestCaseDataResult RequestGetTestCaseData(const FString& TestCaseName, const FContext& Context)
	{
		const TFuture<FGetTestCaseDataResult> Future = RequestGetTestCaseDataAsync(TestCaseName, Context);
		FHttpModule::Get().GetHttpManager().Flush(false);
		check(Future.IsReady());
		return Future.Get();
	}

	//---------------------------------------------------------------------
	// テストケースデータ追加・更新リクエスト
	//---------------------------------------------------------------------
	TFuture<FRequestResult> RequestAddTestCaseDataAsync(const FTestCaseData& TestCaseData, const FContext& Context)
	{
		TSharedPtr<TPromise<FRequestResult>> Promise = MakeShareable(new TPromise<FRequestResult>());
		TFuture<FRequestResult> Future = Promise->GetFuture();

		// Jsonデータの作成
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		TArray<TSharedPtr<FJsonValue>> Commands;
		for (const FString& Command : TestCaseData.GetCommands())
		{
			Commands.Add(MakeShareable(new FJsonValueString(Command)));
		}
		JsonObject->SetArrayField(Internal::FJsonKeys::KeyCommands, Commands);
		JsonObject->SetStringField(Internal::FJsonKeys::KeyName, TestCaseData.GetName());
		JsonObject->SetStringField(Internal::FJsonKeys::KeySummary, TestCaseData.GetSummary());

		// Json書き出し
		FString JsonString;
		TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

		// 送るHTTPリクエスト設定
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

		Request->SetURL(Context.GetURIAddTestCaseData());
		Request->SetVerb(TEXT("POST"));
		Context.AppendBasicAuthHeader(Request);
		Request->SetContentAsString(JsonString);
		Request->SetHeader("Content-Type", TEXT("application/json"));

		auto CompleteFunc = [Promise](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			if (!bConnectedSuccessfully)
			{
				Promise->SetValue(Internal::ConnectFailedResult);
			}
			else
			{
				if (Response->GetResponseCode() == EHttpResponseCodes::Ok)
				{
					const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
					TSharedPtr<FJsonObject> ReadJsonObject = MakeShareable(new FJsonObject());

					FString Message;
					const bool bParseSuccess = FJsonSerializer::Deserialize(JsonReader, ReadJsonObject)
						&& ReadJsonObject->TryGetStringField(Internal::FJsonKeys::KeyMessage, Message);

					if (bParseSuccess)
					{
						Promise->SetValue(FRequestResult(true, Message));
					}
					else
					{
						const FString Msg = FString::Format(TEXT("ResponseParseFailed. Content: {0}"), {*Response->GetContentAsString()});
						Promise->SetValue(FRequestResult(false, Msg));
					}
				}
				else
				{
					Promise->SetValue(FRequestResult(false, Response->GetContentAsString()));
				}
			}
		};

		Request->OnProcessRequestComplete().BindLambda(CompleteFunc);

		Request->ProcessRequest();

		return Future;
	}

	FRequestResult RequestAddTestCaseData(const FTestCaseData& TestCaseData, const FContext& Context)
	{
		const TFuture<FRequestResult> Future = RequestAddTestCaseDataAsync(TestCaseData, Context);
		FHttpModule::Get().GetHttpManager().Flush(false);
		check(Future.IsReady());
		return Future.Get();
	}

	//---------------------------------------------------------------------
	// テストケースデータ名一覧取得リクエスト
	//---------------------------------------------------------------------
	TFuture<FGetTestCaseListResult> RequestGetTestCaseListAsync(const FContext& Context)
	{
		TSharedPtr<TPromise<FGetTestCaseListResult>> Promise = MakeShareable(new TPromise<FGetTestCaseListResult>());
		TFuture<FGetTestCaseListResult> Future = Promise->GetFuture();

		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

		Request->SetURL(Context.GetURIGetTestCaseNameList());
		Context.AppendBasicAuthHeader(Request);

		auto CompleteFunc = [Promise](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			if (!bConnectedSuccessfully)
			{
				Promise->SetValue(FGetTestCaseListResult(Internal::ConnectFailedResult, TArray<FString>()));
			}
			else
			{
				if (Response->GetResponseCode() == EHttpResponseCodes::Ok)
				{
					const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
					TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

					FString Message;
					TArray<FString> Names;
					const bool bParseSuccess = FJsonSerializer::Deserialize(JsonReader, JsonObject)
						&& JsonObject->TryGetStringField(Internal::FJsonKeys::KeyMessage, Message)
						&& JsonObject->TryGetStringArrayField(Internal::FJsonKeys::KeyTestCases, Names);
					if (bParseSuccess)
					{
						Promise->SetValue(TTuple<FRequestResult, TArray<FString>>(FRequestResult(true, Message), Names));
					}
					else
					{
						const FString Msg = FString::Format(TEXT("ResponseParseFailed. Content: {0}"), {*Response->GetContentAsString()});
						Promise->SetValue(FGetTestCaseListResult(FRequestResult(false, Msg), TArray<FString>()));
					}
				}
				else
				{
					Promise->SetValue(FGetTestCaseListResult(FRequestResult(false, Response->GetContentAsString()), TArray<FString>()));
				}
			}
		};

		Request->OnProcessRequestComplete().BindLambda(CompleteFunc);

		Request->ProcessRequest();

		return Future;
	}

	FGetTestCaseListResult RequestGetTestCaseList(const FContext& Context)
	{
		const TFuture<FGetTestCaseListResult> Future = RequestGetTestCaseListAsync(Context);
		FHttpModule::Get().GetHttpManager().Flush(false);
		check(Future.IsReady());
		return Future.Get();
	}
}
