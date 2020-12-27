// Fill out your copyright notice in the Description page of Project Settings.


#include "AutomationSampleSystem.h"
#include "TestHostingAPI.h"
#include "SampleMisc.h"


DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FStartAutomationCommand, FString, TestCaseName);

bool FStartAutomationCommand::Update()
{
	UWorld* World = SampleMisc::GetAnyGameWorld();
	if (World != nullptr)
	{
		World->GetGameInstance()->GetSubsystem<UAutomationSampleSubSystem>()->Play(TestCaseName);
	}
	return true;
}


DEFINE_LATENT_AUTOMATION_COMMAND(FWaitAutomationCommand);

bool FWaitAutomationCommand::Update()
{
	UWorld* World = SampleMisc::GetAnyGameWorld();
	if (World != nullptr)
	{
		return !World->GetGameInstance()->GetSubsystem<UAutomationSampleSubSystem>()->IsBusy();
	}
	else
	{
		return true;
	}
}

// セッションフロントのAutomation向け
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FIntegrateUEAutomationFrameworkSample, "IntegrateAutomationSample",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
	EAutomationTestFlags::ProductFilter)

void FIntegrateUEAutomationFrameworkSample::GetTests(TArray<FString>& OutBeautifiedNames,
													 TArray<FString>& OutTestCommands) const
{
	TestHosting::FGetTestCaseListRequest GetRequest;

	if (GetRequest.Request(SampleMisc::Context).IsSuccess())
	{
		const TArray<FString>& Names = GetRequest.GetTestCaseNames();
		for (const FString& Name : Names)
		{
			OutBeautifiedNames.Add(Name);
			OutTestCommands.Add(Name);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("テスト一覧取得に失敗しました"));
	}
}

bool FIntegrateUEAutomationFrameworkSample::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FStartAutomationCommand(Parameters));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitAutomationCommand());
	return true;
}
