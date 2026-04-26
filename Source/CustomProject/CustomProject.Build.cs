// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomProject : ModuleRules
{
	public CustomProject(ReadOnlyTargetRules Target) : base(Target)
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
			"UMG",
			"Slate",
			"GameplayTags" ,
			"SlateCore" 
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"CustomProject",
			"CustomProject/Variant_Strategy",
			"CustomProject/Variant_Strategy/UI",
			"CustomProject/Variant_TwinStick",
			"CustomProject/Variant_TwinStick/AI",
			"CustomProject/Variant_TwinStick/Gameplay",
			"CustomProject/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
