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
		  AddURI(FString::Format(TEXT("{0}/add/"), { *ServiceURI })),
	      GetListURI(FString::Format(TEXT("{0}/list/"), { *ServiceURI }))
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
	// APIをテストケースデータ取得リクエスト
	//---------------------------------------------------------------------
	FRequestResult FGetTestCaseDataRequest::Request(const FString& TestCaseName, const FContext& Context)
	{
		Result.Reset();
		TestCaseData.Reset();

		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

		Request->SetURL(Context.GetURIGetTestCaseData(TestCaseName));
		Context.AppendBasicAuthHeader(Request);

		Request->OnProcessRequestComplete().BindRaw(this, &FGetTestCaseDataRequest::OnGetRequestComplete);

		Request->ProcessRequest();

		// HTTPモジュールを強制的に作動させる
		FHttpModule::Get().GetHttpManager().Flush(false);

		// 上記処理によりOnGetRequestCompleteが呼び出されているため結果にアクセスすることができる。
		check(Result.IsSet());

		return Result.GetValue();
	}

	bool FGetTestCaseDataRequest::IsValid() const
	{
		return Result.IsSet() && TestCaseData.IsSet();
	}

	const FRequestResult& FGetTestCaseDataRequest::GetResult() const
	{
		if (ensureAlways(IsValid()))
		{
			return Result.GetValue();
		}
		else
		{
			return Internal::InvalidResult;
		}
	}

	const FTestCaseData& FGetTestCaseDataRequest::GetTestCaseData() const
	{
		if (ensureAlways(IsValid()))
		{
			return TestCaseData.GetValue();
		}
		else
		{
			return Internal::InvalidTestCaseData;
		}
	}

	void FGetTestCaseDataRequest::OnGetRequestComplete(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bConnectedSuccessfully)
	{
		TestCaseData = Internal::InvalidTestCaseData;

		if (!bConnectedSuccessfully)
		{
			Result = Internal::ConnectFailedResult;
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
					Result = FRequestResult(true, Message);
					TestCaseData = FTestCaseData(Name, Commands, Summary);
				}
				else
				{
					const FString Msg = FString::Format(TEXT("ResponseParseFailed. Content: {0}"), {*Response->GetContentAsString()});
					Result = FRequestResult(false, Msg);
				}
			}
			else
			{
				Result = FRequestResult(false, Response->GetContentAsString());
			}
		}
	}

	//---------------------------------------------------------------------
	// APIをテストケースデータ取得リクエスト
	//---------------------------------------------------------------------
	FRequestResult FAddTestCaseDataRequest::Request(const FTestCaseData& TestCaseData, const FContext& Context)
	{
		Result.Reset();

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

		Request->OnProcessRequestComplete().BindRaw(this, &FAddTestCaseDataRequest::OnRequestComplete);

		Request->ProcessRequest();

		// HTTPモジュールを強制的に作動させる
		FHttpModule::Get().GetHttpManager().Flush(false);

		// 上記処理によりOnGetRequestCompleteが呼び出されているため結果にアクセスすることができる。
		check(Result.IsSet());

		return Result.GetValue();
	}

	bool FAddTestCaseDataRequest::IsValid() const
	{
		return Result.IsSet();
	}

	const FRequestResult& FAddTestCaseDataRequest::GetResult() const
	{
		if (ensureAlways(IsValid()))
		{
			return Result.GetValue();
		}
		else
		{
			return Internal::InvalidResult;
		}
	}

	void FAddTestCaseDataRequest::OnRequestComplete(
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bConnectedSuccessfully)
	{
		if (!bConnectedSuccessfully)
		{
			Result = Internal::ConnectFailedResult;
		}
		else
		{
			if (Response->GetResponseCode() == EHttpResponseCodes::Ok)
			{
				const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
				TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

				FString Message;
				const bool bParseSuccess = FJsonSerializer::Deserialize(JsonReader, JsonObject)
					&& JsonObject->TryGetStringField(Internal::FJsonKeys::KeyMessage, Message);

				if (bParseSuccess)
				{
					Result = FRequestResult(true, Message);
				}
				else
				{
					const FString Msg = FString::Format(TEXT("ResponseParseFailed. Content: {0}"), { *Response->GetContentAsString() });
					Result = FRequestResult(false, Msg);
				}
			}
			else
			{
				Result = FRequestResult(false, Response->GetContentAsString());
			}
		}
	}


	//---------------------------------------------------------------------
	// テストケースデータ名一覧取得リクエスト
	//---------------------------------------------------------------------
	const TArray<FString> FGetTestCaseListRequest::InvalidTestCaseNames;

	FRequestResult FGetTestCaseListRequest::Request(const FContext& Context)
	{
		Result.Reset();
		TestCaseNames.Reset();

		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

		Request->SetURL(Context.GetURIGetTestCaseNameList());
		Context.AppendBasicAuthHeader(Request);

		Request->OnProcessRequestComplete().BindRaw(this, &FGetTestCaseListRequest::OnRequestComplete);

		Request->ProcessRequest();

		// HTTPモジュールを強制的に作動させる
		FHttpModule::Get().GetHttpManager().Flush(false);

		// 上記処理によりOnGetRequestCompleteが呼び出されているため結果にアクセスすることができる。
		check(Result.IsSet());

		return Result.GetValue();
	}

	bool FGetTestCaseListRequest::IsValid() const
	{
		return Result.IsSet() && TestCaseNames.IsSet();
	}

	const FRequestResult& FGetTestCaseListRequest::GetResult() const
	{
		if (ensureAlways(IsValid()))
		{
			return Result.GetValue();
		}
		else
		{
			return Internal::InvalidResult;
		}
	}

	const TArray<FString>& FGetTestCaseListRequest::GetTestCaseNames() const
	{
		if (ensureAlways(IsValid()))
		{
			return TestCaseNames.GetValue();
		}
		else
		{
			return InvalidTestCaseNames;
		}
	}

	void FGetTestCaseListRequest::OnRequestComplete(
		FHttpRequestPtr Request, 
		FHttpResponsePtr Response,
		bool bConnectedSuccessfully)
	{
		if (!bConnectedSuccessfully)
		{
			Result = Internal::ConnectFailedResult;
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
				if(bParseSuccess)
				{
					TestCaseNames = Names;
					Result = FRequestResult(true, Message);
				}
				else
				{
					const FString Msg = FString::Format(TEXT("ResponseParseFailed. Content: {0}"), { *Response->GetContentAsString() });
					Result = FRequestResult(false, Msg);
				}
			}
			else
			{
				Result = FRequestResult(false, Response->GetContentAsString());
			}
		}
	}

}
