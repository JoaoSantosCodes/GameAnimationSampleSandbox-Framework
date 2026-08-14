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
				"ModularGameplayActors",
				"SandboxCommon",
				"SandboxInterfaces",
				"SandboxAssets",
				"SandboxCore",
				"AIModule",
				"NetCore",
				"EnhancedInput"
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
