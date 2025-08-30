// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Carnage : ModuleRules
{
	public Carnage(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore", 
				"GameplayTasks",
				"GameplayTags",
				"HeadMountedDisplay",
				"NavigationSystem",
				"AIModule",
				"Niagara",
				"PhysicsCore",
				"Landscape" });
    }
}
