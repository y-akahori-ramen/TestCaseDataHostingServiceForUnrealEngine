// Fill out your copyright notice in the Description page of Project Settings.


#include "SampleConsoleCommands.h"
#include "AutomationSampleSystem.h"
#include "SampleMisc.h"
#include "TestHostingAPI.h"

void FSampleConsoleCommands::GetTestCaseData(const FString& TestCaseName)
{
	const TestHosting::FGetTestCaseDataResult Result = TestHosting::RequestGetTestCaseData(TestCaseName, SampleMisc::Context);

	if (Result.Key.IsSuccess())
	{
		const TestHosting::FTestCaseData& Data = Result.Value;
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
		UE_LOG(LogTemp, Error, TEXT("Failure %s"), *Result.Key.GetMessage());
	}
}

void FSampleConsoleCommands::GetTestCaseDataAsync(const FString& TestCaseName)
{
	UWorld* World = SampleMisc::GetAnyGameWorld();
	if (World != nullptr)
	{
		World->GetGameInstance()->GetSubsystem<UAutomationSampleSubSystem>()->GetTestCaseDataAsync(TestCaseName);
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

	const TestHosting::FRequestResult Result = TestHosting::RequestAddTestCaseData(SampleTestCaseData, SampleMisc::Context);
	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Success: %s"), *Result.GetMessage());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failure %s"), *Result.GetMessage());
	}
}

void FSampleConsoleCommands::AddSampleTestCaseDataAsync(const FString& TestCaseName)
{
	UWorld* World = SampleMisc::GetAnyGameWorld();
	if (World != nullptr)
	{
		World->GetGameInstance()->GetSubsystem<UAutomationSampleSubSystem>()->AddSampleTestCaseDataAsync(TestCaseName);
	}
}

void FSampleConsoleCommands::GetTestCaseNameList()
{
	const TestHosting::FGetTestCaseListResult Result = TestHosting::RequestGetTestCaseList(SampleMisc::Context);

	if(Result.Key.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("Success: %s"), *Result.Key.GetMessage());
		for (const FString& Name : Result.Value)
		{
			UE_LOG(LogTemp, Log, TEXT("TestCaseName: %s"), *Name);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failure %s"), *Result.Key.GetMessage());
	}
}

void FSampleConsoleCommands::GetTestCaseNameListAsync()
{
	UWorld* World = SampleMisc::GetAnyGameWorld();
	if (World != nullptr)
	{
		World->GetGameInstance()->GetSubsystem<UAutomationSampleSubSystem>()->GetTestCaseListAsync();
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

