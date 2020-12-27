// Fill out your copyright notice in the Description page of Project Settings.


#include "SampleConsoleCommands.h"
#include "AutomationSampleSystem.h"
#include "SampleMisc.h"
#include "TestHostingAPI.h"

void FSampleConsoleCommands::GetTestCaseData(const FString& TestCaseName)
{
	TestHosting::FGetTestCaseDataRequest GetRequest;

	if (GetRequest.Request(TestCaseName, SampleMisc::Context).IsSuccess())
	{
		const TestHosting::FTestCaseData& Data = GetRequest.GetTestCaseData();
		UE_LOG(LogTemp, Log, TEXT("Name: %s"), *Data.GetName());
		UE_LOG(LogTemp, Log, TEXT("Summary: %s"), *Data.GetSummary());
		UE_LOG(LogTemp, Log, TEXT("Start Dump Commands"));
		for (const FString& Command : Data.GetCommands())
		{
			UE_LOG(LogTemp, Log, TEXT("Command: %s"), *Command);
		}
		UE_LOG(LogTemp, Log, TEXT("End Dump Commands"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failure %s"), *GetRequest.GetResult().GetMessage());
	}
}

void FSampleConsoleCommands::AddSampleTestCaseData(const FString& TestCaseName)
{
	const FString SampleSummaryText(TEXT("Summary Sample"));

	TArray<FString> SampleCommands;
	SampleCommands.Add(TEXT("SampleCommand1"));
	SampleCommands.Add(TEXT("SampleCommand2"));
	SampleCommands.Add(TEXT("SampleCommand3"));
	SampleCommands.Add(TEXT("SampleCommand4"));
	SampleCommands.Add(TEXT("SampleCommand5"));

	const TestHosting::FTestCaseData SampleTestCaseData(TestCaseName, SampleCommands, SampleSummaryText);

	TestHosting::FAddTestCaseDataRequest AddRequest;
	if (AddRequest.Request(SampleTestCaseData, SampleMisc::Context).IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Success: %s"), *AddRequest.GetResult().GetMessage());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failure %s"), *AddRequest.GetResult().GetMessage());
	}
}

void FSampleConsoleCommands::GetTestCaseNameList()
{
	TestHosting::FGetTestCaseListRequest GetListRequest;
	if (GetListRequest.Request(SampleMisc::Context).IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Success: %s"), *GetListRequest.GetResult().GetMessage());
		for (const FString& Name : GetListRequest.GetTestCaseNames())
		{
			UE_LOG(LogTemp, Log, TEXT("TestCaseName: %s"), *Name);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failure %s"), *GetListRequest.GetResult().GetMessage());
	}
}

void FSampleConsoleCommands::StartRecording()
{
	UWorld* World = SampleMisc::GetAnyGameWorld();
	if(World != nullptr)
	{
		World->GetGameInstance()->GetSubsystem<UAutomationSampleSubSystem>()->StartRecording();
	}
}

void FSampleConsoleCommands::StopRecording(const FString& TestCaseName)
{
	UWorld* World = SampleMisc::GetAnyGameWorld();
	if (World != nullptr)
	{
		World->GetGameInstance()->GetSubsystem<UAutomationSampleSubSystem>()->StopRecording(TestCaseName);
	}
}

void FSampleConsoleCommands::PlayRecordingData(const FString& TestCaseName)
{
	UWorld* World = SampleMisc::GetAnyGameWorld();
	if (World != nullptr)
	{
		World->GetGameInstance()->GetSubsystem<UAutomationSampleSubSystem>()->Play(TestCaseName);
	}
}

