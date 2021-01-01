// Fill out your copyright notice in the Description page of Project Settings.


#include "AutomationSampleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "TestHostingAPI.h"
#include "SampleMisc.h"

//---------------------------------------------------------------------------------
// UAutomationSampleSubSystem::FCommand
// 記録する歩きデータ
//---------------------------------------------------------------------------------
class UAutomationSampleSubSystem::FCommand final
{
public:
	FCommand(FVector Pos, bool bTeleport)
		: Pos(Pos), bTeleport(bTeleport)
	{
	}

	explicit FCommand(const FString& CmdText)
	{
		const FRegexPattern MatchPattern = FRegexPattern(CmdRegexPattern);

		FRegexMatcher Matcher(MatchPattern, CmdText);

		if (Matcher.FindNext())
		{
			bTeleport = Matcher.GetCaptureGroup(1) == TEXT("1");
			Pos.InitFromString(Matcher.GetCaptureGroup(2));
		}
	}

	FString ToString() const
	{
		return FString::Format(TEXT("bTeleport:{0} Pos:{1}"), {bTeleport, *Pos.ToString()});
	}

	static bool IsValidCommand(const FString& CmdText)
	{
		const FRegexPattern MatchPattern = FRegexPattern(CmdRegexPattern);
		FRegexMatcher Matcher(MatchPattern, CmdText);
		return Matcher.FindNext();
	}

	const FVector& GetPos() const { return Pos; }
	bool IsTeleport() const { return bTeleport; }

private:
	FVector Pos;
	bool bTeleport;
	static const FString CmdRegexPattern;
};

const FString UAutomationSampleSubSystem::FCommand::CmdRegexPattern(TEXT(R"(bTeleport:(\d)\sPos:(.+))"));


//---------------------------------------------------------------------------------
// UAutomationSampleSubSystem::FRecorder
// 一定距離ごとに歩きデータを記録する
//---------------------------------------------------------------------------------
class UAutomationSampleSubSystem::FRecorder final
{
public:
	explicit FRecorder(const UObject* WorldContextObject)
		: WorldContextObject(WorldContextObject)
	{
	}

	void Start()
	{
		UE_LOG(LogTemp, Log, TEXT("Start recording"));

		Commands.Empty();

		const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
		bIsBusy = PlayerPawn != nullptr;
		if (bIsBusy)
		{
			PrevRecordPos = PlayerPawn->GetActorLocation();
			const TSharedPtr<FCommand> Command(MakeShareable(new FCommand(PrevRecordPos, /*bTeleport=*/ true)));
			Commands.Add(Command);
			UE_LOG(LogTemp, Log, TEXT("Record %s"), *Command->ToString());
		}
	}

	void Stop(const FString& TestCaseDataName)
	{
		if (bIsBusy)
		{
			UE_LOG(LogTemp, Log, TEXT("Stop recording and send recording data to hosting service."));

			// 録画したコマンドをテストケースデータに変換して保存
			const FString SampleSummaryText(
				FString::Format(TEXT("WalkTestData record at {0}"), {*FDateTime::Now().ToString()}));

			UE_LOG(LogTemp, Log, TEXT("Name: %s"), *TestCaseDataName);
			UE_LOG(LogTemp, Log, TEXT("Summary: %s"), *SampleSummaryText);
			UE_LOG(LogTemp, Log, TEXT("Commands:"));
			TArray<FString> WalkCommands;
			for (const TSharedPtr<FCommand> Command : Commands)
			{
				UE_LOG(LogTemp, Log, TEXT("%s"), *Command->ToString());
				WalkCommands.Add(Command->ToString());
			}

			const TestHosting::FTestCaseData SampleTestCaseData(TestCaseDataName, WalkCommands, SampleSummaryText);

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
		bIsBusy = false;
	}

	bool Update(float deltaTime)
	{
		const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
		bIsBusy = bIsBusy && (PlayerPawn != nullptr);
		if (bIsBusy)
		{
			constexpr float RecordIntervalDistSeq = 100.0f * 100.0f;
			const float DistFromPrevRecordPosSeq = FVector::DistSquaredXY(PrevRecordPos, PlayerPawn->GetActorLocation());
			const bool bRecord = DistFromPrevRecordPosSeq >= RecordIntervalDistSeq;
			if (bRecord)
			{
				PrevRecordPos = PlayerPawn->GetActorLocation();
				const TSharedPtr<FCommand> Command(MakeShareable(new FCommand(PrevRecordPos, /*bTeleport=*/ false)));
				Commands.Add(Command);
				UE_LOG(LogTemp, Log, TEXT("Record %s"), *Command->ToString());
			}
		}

		return bIsBusy;
	}

	bool IsBusy() const
	{
		return bIsBusy;
	}

private:
	const UObject* const WorldContextObject;
	bool bIsBusy = false;
	FVector PrevRecordPos;
	TArray<TSharedPtr<FCommand>> Commands;
};

//---------------------------------------------------------------------------------
// UAutomationSampleSubSystem::FPlayer
// 保存したテストデータからFCommandを復元し、再現する
//---------------------------------------------------------------------------------
class UAutomationSampleSubSystem::FPlayer final
{
public:
	explicit FPlayer(const UObject* WorldContextObject, const FString& TestCaseName)
		: WorldContextObject(WorldContextObject)
	{
		UE_LOG(LogTemp, Log, TEXT("Play recording data. name: %s"), *TestCaseName);

		const TestHosting::FGetTestCaseDataResult Result = TestHosting::RequestGetTestCaseData(TestCaseName, SampleMisc::Context);
		if (Result.Key.IsSuccess())
		{
			const TestHosting::FTestCaseData& TestCaseData = Result.Value;
			for (const FString& Cmd : TestCaseData.GetCommands())
			{
				if (FCommand::IsValidCommand(Cmd))
				{
					Commands.Enqueue(MakeShareable(new FCommand(Cmd)));
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid format. Cmd: %s"), *Cmd);
					Commands.Empty();
					break;
				}
			}
		}
	}

	bool IsBusy() const
	{
		return !Commands.IsEmpty() || ActiveCommand.IsValid();
	}

	void Update(float DeltaTime)
	{
		if (!IsBusy())
		{
			return;
		}

		if (ActiveCommand.IsValid())
		{
			MoveElapsedTime += DeltaTime;
			
			APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContextObject, 0);
			if(ActiveCommand->IsTeleport())
			{
				PlayerPawn->TeleportTo(ActiveCommand->GetPos(), PlayerPawn->GetActorRotation());
				ActiveCommand.Reset();
			}
			else
			{
				constexpr float MoveCompleteThresholdSeq = 10 * 10;
				constexpr float TimeOutSec = 5.0f;
				const float DistanceXYSeq = FVector::DistSquaredXY(ActiveCommand->GetPos(), PlayerPawn->GetActorLocation());
				const bool bMoveComplete = DistanceXYSeq <= MoveCompleteThresholdSeq;
				const bool bTimeOut = MoveElapsedTime > TimeOutSec;
				if (!bMoveComplete && !bTimeOut)
				{
					FVector MoveVector(ActiveCommand->GetPos() - PlayerPawn->GetActorLocation());
					MoveVector.Z = 0;
					MoveVector.Normalize();
					PlayerPawn->AddMovementInput(MoveVector);
				}
				else
				{
					if(bTimeOut)
					{
						UE_LOG(LogTemp, Warning, TEXT("Time out"));
					}

					if(!FMath::IsNearlyZero(DistanceXYSeq))
					{
						PlayerPawn->TeleportTo(ActiveCommand->GetPos(), PlayerPawn->GetActorRotation());
					}
					
					ActiveCommand.Reset();
				}
			}

			if(!IsBusy())
			{
				UE_LOG(LogTemp, Log, TEXT("Complete."))
			}
		}
		else if(!Commands.IsEmpty())
		{
			Commands.Dequeue(ActiveCommand);
			MoveElapsedTime = 0;
			UE_LOG(LogTemp, Log, TEXT("StartCmd: %s"), *ActiveCommand->ToString());
		}
	}

private:
	const UObject* const WorldContextObject;
	TQueue<TSharedPtr<FCommand>> Commands;
	TSharedPtr<FCommand> ActiveCommand;
	bool bIsBusy = false;
	float MoveElapsedTime = 0;
};

//---------------------------------------------------------------------------------
// UAutomationSampleSubSystem
//---------------------------------------------------------------------------------
TStatId UAutomationSampleSubSystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAutomationSampleSubSystem, STATGROUP_Tickables);
}

void UAutomationSampleSubSystem::Tick(float DeltaTime)
{
	if (Recorder.IsValid())
	{
		Recorder->Update(DeltaTime);
	}
	if (Player.IsValid())
	{
		Player->Update(DeltaTime);
	}
}

bool UAutomationSampleSubSystem::IsTickable() const
{
	return IsBusy();
}

void UAutomationSampleSubSystem::StartRecording()
{
	Recorder = MakeShareable(new FRecorder(this));
	Recorder->Start();
}

void UAutomationSampleSubSystem::StopRecording(const FString& TestCaseDataName)
{
	if (Recorder.IsValid())
	{
		Recorder->Stop(TestCaseDataName);
	}
	Recorder.Reset();
}

void UAutomationSampleSubSystem::Play(const FString& TestCaseDataName)
{
	Player = MakeShareable(new FPlayer(this, TestCaseDataName));
}

bool UAutomationSampleSubSystem::IsBusy() const
{
	return (Recorder.IsValid() && Recorder->IsBusy())
		|| (Player.IsValid() && Player->IsBusy());
}
