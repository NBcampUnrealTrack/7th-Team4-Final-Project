// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Pentagram : ModuleRules
{
	public Pentagram(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"Slate",
            "GameplayTags",

            //UI
            "UMG",
            "CommonUI",
            "CommonInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
        {
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
        });

		PublicIncludePaths.AddRange(new string[] {
			"Pentagram",
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
