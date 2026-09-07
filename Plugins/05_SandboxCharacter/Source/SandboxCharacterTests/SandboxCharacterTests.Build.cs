using UnrealBuildTool;

// Modulo de testes: tipo UncookedOnly — nunca entra em build cozinhado, logo nunca vai
// no produto do comprador. E a convencao dominante da propria engine para suites de teste.
public class SandboxCharacterTests : ModuleRules
{
	public SandboxCharacterTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
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
				"PhysicsCore",
				"SandboxCharacter"
			}
			);
	}
}
