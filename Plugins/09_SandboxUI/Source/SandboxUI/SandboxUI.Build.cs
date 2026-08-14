using UnrealBuildTool;

public class SandboxUI : ModuleRules
{
	public SandboxUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"UMG",
				"Slate",
				"SlateCore",
				"GameplayTags",
				"ModularGameplay",
				"SandboxCommon",
				"SandboxInterfaces",
				"SandboxAssets",
				"SandboxCore"
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
