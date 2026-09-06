using UnrealBuildTool;

public class SandboxAssets : ModuleRules
{
	public SandboxAssets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"UMG",
				"SandboxCommon",
				"SandboxInterfaces",
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
