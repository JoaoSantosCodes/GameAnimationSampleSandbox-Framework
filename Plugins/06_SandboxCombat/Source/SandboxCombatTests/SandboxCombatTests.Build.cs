using UnrealBuildTool;

// Modulo de testes: tipo UncookedOnly — nunca entra em build cozinhado, logo nunca vai
// no produto do comprador. E a convencao dominante da propria engine para suites de teste.
public class SandboxCombatTests : ModuleRules
{
	public SandboxCombatTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"GameplayTags",
				"SandboxCommon",
				"SandboxInterfaces",
				"SandboxCore",
				"SandboxCharacter",
				"ModularGameplay",
				"AIModule",
				"SmartObjectsModule",
				"StateTreeModule",
				"GameplayStateTreeModule",
				"SandboxCombat"
			}
			);
	}
}
