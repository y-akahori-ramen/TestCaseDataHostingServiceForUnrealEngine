// Fill out your copyright notice in the Description page of Project Settings.

#include "SampleMisc.h"

namespace SampleMisc
{
	const TestHosting::FContext Context(TEXT("admin"), TEXT("admin"), TEXT("http://localhost/"));

	UWorld* GetAnyGameWorld()
	{
		UWorld* TestWorld = nullptr;
		const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
		for (const FWorldContext& WorldContext : WorldContexts)
		{
			if (((WorldContext.WorldType == EWorldType::PIE) || (WorldContext.WorldType == EWorldType::Game)) && (WorldContext.World() != nullptr))
			{
				TestWorld = WorldContext.World();
				break;
			}
		}

		return TestWorld;
	}
}
