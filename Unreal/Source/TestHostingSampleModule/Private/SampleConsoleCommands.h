// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * TestCaseHostingServiceプラグインの利用サンプルコマンド
 */
class FSampleConsoleCommands
{
public:
	/**
	 * @brief 指定したテストケース名のデータを取得し内容をログに出力する
	 * @param TestCaseName テストケース名
	 */
	static void GetTestCaseData(const FString& TestCaseName);

	/**
	 * @brief 指定したテストケース名のデータを取得し内容をログに出力する　非同期
	 * @param TestCaseName テストケース名
	 */
	static void GetTestCaseDataAsync(const FString& TestCaseName);

	/**
	 * @brief 指定したテストケース名のサンプルデータを追加する
	 * @param TestCaseName テストケース名
	 */
	static void AddSampleTestCaseData(const FString& TestCaseName);

	/**
	 * @brief 指定したテストケース名のサンプルデータを追加する 非同期版
	 * @param TestCaseName テストケース名
	 */
	static void AddSampleTestCaseDataAsync(const FString& TestCaseName);
	
	/**
	 * @brief テストケース名一覧をログに出力する
	 */
	static void GetTestCaseNameList();

	/**
	 * @brief テストケース名一覧をログに出力する 非同期版
	 */
	static void GetTestCaseNameListAsync();

	/**
	 * @brief 歩きデータの記録を開始する
	 */
	static void StartRecording();

	
	/**
	 * @brief 歩きデータの記録を終了し、指定した名前でデータを保存する
	 * @param TestCaseName 保存するデータ名
	 */
	static void StopRecording(const FString& TestCaseName);


	/**
	 * @brief 保存済みの指定した名前のデータを再生する
	 * @param TestCaseName 再生するデータ名
	 */
	static void PlayRecordingData(const FString& TestCaseName);
};
