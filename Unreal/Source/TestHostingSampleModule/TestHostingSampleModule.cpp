// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestHostingSampleModule.h"
#include "Modules/ModuleManager.h"
#include "Private/SampleConsoleCommands.h"

class FTestHostingSampleModule final : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
};


void FTestHostingSampleModule::StartupModule()
{
	if (!IsRunningCommandlet())
	{
		// TestHostingServiceプラグインのサンプルコンソールコマンド登録
		
		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("GetTestCaseData"),
			TEXT("GetTestCaseData TestCaseName"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
			{
				if(Args.Num() != 1)
				{
					UE_LOG(LogTemp, Error, TEXT("Command format error"));
				}
				else
				{
					FSampleConsoleCommands::GetTestCaseData(Args[0]);
				}
			}),
			ECVF_Default
		);

		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("AddTestCaseData"),
			TEXT("AddTestCaseData TestCaseName"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
				{
					//&AddSampleTestCaseDataCommand
					if (Args.Num() != 1)
					{
						UE_LOG(LogTemp, Error, TEXT("Command format error"));
					}
					else
					{
						FSampleConsoleCommands::AddSampleTestCaseData(Args[0]);
					}
				}),
			ECVF_Default
		);

		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("GetTestCaseList"),
			TEXT("GetTestCaseList"),
			FConsoleCommandDelegate::CreateLambda([] { FSampleConsoleCommands::GetTestCaseNameList(); }),
			ECVF_Default
		);

		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("StartRecording"),
			TEXT("StartRecording"),
			FConsoleCommandDelegate::CreateLambda([] { FSampleConsoleCommands::StartRecording(); }),
			ECVF_Default
		);

		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("StopRecording"),
			TEXT("StopRecording TestCaseName"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
			{
				//&AddSampleTestCaseDataCommand
				if (Args.Num() != 1)
				{
					UE_LOG(LogTemp, Error, TEXT("Command format error"));
				}
				else
				{
					FSampleConsoleCommands::StopRecording(Args[0]);
				}
			}),
			ECVF_Default
		);

		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("PlayRecordingData"),
			TEXT("PlayRecordingData TestCaseName"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
			{
				//&AddSampleTestCaseDataCommand
				if (Args.Num() != 1)
				{
					UE_LOG(LogTemp, Error, TEXT("Command format error"));
				}
				else
				{
					FSampleConsoleCommands::PlayRecordingData(Args[0]);
				}
			}),
			ECVF_Default
		);
	}
}


IMPLEMENT_GAME_MODULE(FTestHostingSampleModule, TestHostingSampleModule);
