// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestHostingSampleModule.h"
#include "Modules/ModuleManager.h"
#include "TestHostingService/Public/TestHostingAPI.h"

class FTestHostingSampleModule final : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
};


static void GetTestCaseDataCommand(const TArray<FString>& Args)
{
	if (Args.Num() != 1)
	{
		UE_LOG(LogTemp, Error, TEXT("Command format error"));
	}
	else
	{
		const TestHosting::FContext Context(TEXT("admin"), TEXT("admin"), TEXT("http://localhost/"));
		TestHosting::FGetTestCaseDataRequest GetRequest;

		const FString TestCaseName(Args[0]);
		if (GetRequest.Request(TestCaseName, Context).IsSuccess())
		{
			const TestHosting::FTestCaseData& Data = GetRequest.GetTestCaseData();
			UE_LOG(LogTemp, Log, TEXT("Name: %s"), *Data.GetName());
			UE_LOG(LogTemp, Log, TEXT("Summary: %s"), *Data.GetSummary());
			UE_LOG(LogTemp, Log, TEXT("Start Dump Commands"));
			for (const FString& Command : Data.GetCommands())
			{
				UE_LOG(LogTemp, Log, TEXT("Cms: %s"), *Command);
			}
			UE_LOG(LogTemp, Log, TEXT("End Dump Commands"));
		}
		else
		{
			UE_LOG(LogTemp, Error,TEXT("Failure %s"), *GetRequest.GetResult().GetMessage());
		}
	}


	for (const FString& Arg : Args)
	{
		UE_LOG(LogTemp, Log, TEXT("Arg: %s"), *Arg);
	}
}

static void AddSampleTestCaseDataCommand(const TArray<FString>& Args)
{
	if(Args.Num() != 1)
	{
		UE_LOG(LogTemp, Error, TEXT("Command format error"));
	}
	else
	{
		const FString TestCaseName(Args[0]);
		const FString SampleSummaryText(TEXT("Summary Sample"));
		TArray<FString> SampleCommands;
		SampleCommands.Add(TEXT("SampleCommand1"));
		SampleCommands.Add(TEXT("SampleCommand2"));
		SampleCommands.Add(TEXT("SampleCommand3"));
		SampleCommands.Add(TEXT("SampleCommand4"));
		SampleCommands.Add(TEXT("SampleCommand5"));
		const TestHosting::FTestCaseData SampleTestCaseData(TestCaseName, SampleCommands, SampleSummaryText);

		const TestHosting::FContext Context(TEXT("admin"), TEXT("admin"), TEXT("http://localhost/"));
		TestHosting::FAddTestCaseDataRequest AddRequest;
		if (AddRequest.Request(SampleTestCaseData, Context).IsSuccess())
		{
			UE_LOG(LogTemp, Log, TEXT("Success: %s"), *AddRequest.GetResult().GetMessage());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failure %s"), *AddRequest.GetResult().GetMessage());
		}
	}
}

static void GetTestCaseNameList(const TArray<FString>& Args)
{
	const TestHosting::FContext Context(TEXT("admin"), TEXT("admin"), TEXT("http://localhost/"));
	TestHosting::FGetTestCaseListRequest GetListRequest;
	if (GetListRequest.Request(Context).IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Success: %s"), *GetListRequest.GetResult().GetMessage());
		for(const FString& Name : GetListRequest.GetTestCaseNames())
		{
			UE_LOG(LogTemp, Log, TEXT("TestCaseName: %s"), *Name);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failure %s"), *GetListRequest.GetResult().GetMessage());
	}
}

void FTestHostingSampleModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("FTestHostingSampleModule::StartupModule"));
	if (!IsRunningCommandlet())
	{
		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("GetTestCaseData"),
			TEXT("GetTestCaseData TestCaseName"),
			FConsoleCommandWithArgsDelegate::CreateStatic(&GetTestCaseDataCommand),
			ECVF_Default
		);

		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("AddTestCaseData"),
			TEXT("AddTestCaseData TestCaseName"),
			FConsoleCommandWithArgsDelegate::CreateStatic(&AddSampleTestCaseDataCommand),
			ECVF_Default
		);

		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("GetTestCaseList"),
			TEXT("GetTestCaseList TestCaseName"),
			FConsoleCommandWithArgsDelegate::CreateStatic(&GetTestCaseNameList),
			ECVF_Default
		);
	}
}


IMPLEMENT_GAME_MODULE(FTestHostingSampleModule, TestHostingSampleModule);
