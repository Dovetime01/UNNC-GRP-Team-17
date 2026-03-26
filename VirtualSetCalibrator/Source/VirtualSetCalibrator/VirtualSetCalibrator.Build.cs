// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VirtualSetCalibrator : ModuleRules
{
	public VirtualSetCalibrator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core"
			});

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"DesktopPlatform",
				"EditorFramework",
				"Engine",
				"ImageWrapper",
				"InputCore",
				"NNE",
				"NNERuntimeORT",
				"Projects",
				"Slate",
				"SlateCore",
				"ToolMenus",
				"UnrealEd"
			});
	}
}
