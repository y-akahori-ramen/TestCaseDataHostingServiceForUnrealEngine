// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AutomationSampleSystem.generated.h"

/**
 * TestHostingServiceを使用した自動テストサンプル
 *
 * Third Person Templateで歩いたプレイヤーの情報を保存し、保存されたデータから歩きを再現する 
 */
UCLASS()
class UAutomationSampleSubSystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
public:
	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;

	/**
	 * @brief プレイヤーの歩きデータを記録する
	 */
	void StartRecording();


	/**
	 * @brief 記録を終了し、記録したデータを指定の名前で保存する
	 * @param TestCaseDataName 保存する名前
	 */
	void StopRecording(const FString& TestCaseDataName);

	/**
	 * @brief 保存済みの指定した名前の歩きデータを再生する
	 * @param TestCaseName 再生するデータ名
	 */
	void Play(const FString& TestCaseDataName);


	/**
	 * @brief 記録もしくは再生が行われているか
	 */
	bool IsBusy() const;
private:
	class FCommand;
	class FRecorder;
	class FPlayer;

	TSharedPtr<FRecorder> Recorder;
	TSharedPtr<FPlayer> Player;
};
