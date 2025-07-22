// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CarnageEditorTarget : TargetRules
{
	public CarnageEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("Carnage");
        WindowsPlatform.CompilerVersion = "14.38.33130";
        //WindowsPlatform.Compiler = WindowsCompiler.Clang; // Nur wenn du Clang wirklich nutzt
    }
}
