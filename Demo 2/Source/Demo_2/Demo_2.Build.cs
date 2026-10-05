// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Demo_2 : ModuleRules
{
	public Demo_2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UtilityDialogueSystemHamo", "UMG", "SlateCore", "MovieScene", "LevelSequence" });
	}
}
