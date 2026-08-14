// Build Touch for SBSaveTests
using UnrealBuildTool;

public class SandboxCore : ModuleRules
{
	public SandboxCore(ReadOnlyTargetRules Target) : base(Target)
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
				"EnhancedInput",
				"SandboxCommon",
				"SandboxInterfaces",
				"SandboxAssets"
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
