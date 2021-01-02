// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TestHostingAPI.h"


namespace SampleMisc
{
	extern const TestHosting::FContext Context;

	UWorld* GetAnyGameWorld();
}
