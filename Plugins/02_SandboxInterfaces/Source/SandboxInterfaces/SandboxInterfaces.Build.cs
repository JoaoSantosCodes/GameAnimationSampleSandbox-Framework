using UnrealBuildTool;

public class SandboxInterfaces : ModuleRules
{
	public SandboxInterfaces(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags"
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
