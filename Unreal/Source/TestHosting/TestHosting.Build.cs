// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TestHosting : ModuleRules
{
	public TestHosting(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
