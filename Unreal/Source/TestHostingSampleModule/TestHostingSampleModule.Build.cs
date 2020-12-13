// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TestHostingSampleModule : ModuleRules
{
	public TestHostingSampleModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });

		// TestHostingService�v���O�C����C++���g�p�ł���悤�ɂ���
		PublicDependencyModuleNames.Add("TestHostingService");
	}
}
