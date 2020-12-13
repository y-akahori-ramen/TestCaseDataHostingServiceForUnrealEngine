// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TestHostingEditorTarget : TargetRules
{
	public TestHostingEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("TestHosting");
		ExtraModuleNames.Add("TestHostingSampleModule");
	}
}
