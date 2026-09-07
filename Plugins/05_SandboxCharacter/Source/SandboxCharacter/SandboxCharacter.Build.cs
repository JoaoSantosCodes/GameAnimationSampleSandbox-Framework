// Copyright 2026 João Santos. All Rights Reserved.
// Touch for SBSaveTests
using UnrealBuildTool;

public class SandboxCharacter : ModuleRules
{
	public SandboxCharacter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"ModularGameplay",
				"SandboxCommon",
				"SandboxInterfaces",
				"SandboxAssets",
				"SandboxCore",
				"AIModule",
				"NetCore",
				"EnhancedInput",
				"PhysicsCore"
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ...
			}
			);
	}
}
