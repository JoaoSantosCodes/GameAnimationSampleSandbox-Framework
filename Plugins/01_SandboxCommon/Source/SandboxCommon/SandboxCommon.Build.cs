using UnrealBuildTool;

public class SandboxCommon : ModuleRules
{
	public SandboxCommon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"DeveloperSettings",
				"SandboxInterfaces",
				"ModularGameplay"
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
