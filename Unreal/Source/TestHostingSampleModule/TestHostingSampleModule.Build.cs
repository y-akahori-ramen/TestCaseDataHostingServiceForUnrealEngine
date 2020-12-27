// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TestHostingSampleModule : ModuleRules
{
	public TestHostingSampleModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });

		// TestHostingServiceプラグインのC++を使用できるようにする
		PublicDependencyModuleNames.Add("TestHostingService");
	}
}
