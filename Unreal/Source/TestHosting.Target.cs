// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TestHostingTarget : TargetRules
{
	public TestHostingTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.Add("TestHosting");
        ExtraModuleNames.Add("TestHostingSampleModule");
    }
}
