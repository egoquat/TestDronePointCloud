// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class DronePointCloud01 : ModuleRules
{
	public DronePointCloud01(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"Json", "ImageWriteQueue", "UMG", "RenderCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
		
		PrivateIncludePaths .AddRange(new string[] { "DronePointCloud01" });
		
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, ""),
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, ""),
			}
		);
	}
}
