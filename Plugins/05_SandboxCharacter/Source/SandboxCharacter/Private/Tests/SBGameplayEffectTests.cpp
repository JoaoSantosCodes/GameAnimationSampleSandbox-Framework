#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/SBGameplayEffectComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBGameplayEffectTestsSpec, "Sandbox.Combat.GameplayEffects", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBAttributeComponent* AttributeComp;
	USBStateComponent* StateComp;
	USBGameplayEffectComponent* EffectComp;
END_DEFINE_SPEC(FSBGameplayEffectTestsSpec)

void FSBGameplayEffectTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		TestActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		// Um AActor puro nao tem RootComponent: sem ele, SetActorLocation, SetActorTransform
		// e TeleportTo falham em silencio e o ator fica preso na origem.
		USceneComponent* TestActorRoot = NewObject<USceneComponent>(TestActor, TEXT("TestActorRoot"));
		TestActor->SetRootComponent(TestActorRoot);
		TestActorRoot->RegisterComponent();

		AttributeComp = NewObject<USBAttributeComponent>(TestActor, TEXT("TestAttributes"));
		AttributeComp->RegisterComponent();
		TestActor->AddOwnedComponent(AttributeComp);

		StateComp = NewObject<USBStateComponent>(TestActor, TEXT("TestState"));
		StateComp->RegisterComponent();
		TestActor->AddOwnedComponent(StateComp);

		EffectComp = NewObject<USBGameplayEffectComponent>(TestActor, TEXT("TestEffects"));
		EffectComp->RegisterComponent();
		TestActor->AddOwnedComponent(EffectComp);

		ISBComponentInterface::Execute_OnInitialize(AttributeComp);
		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(EffectComp);

		// Registra atributo de teste (Vida base = 100)
		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.0f;
		HealthAttr.CurrentValue = 100.0f;
		HealthAttr.MaxValue = 200.0f;
		HealthAttr.MinValue = 0.0f;
		AttributeComp->RegisterAttribute(FSBGameplayTags::Get().Attribute_Health, HealthAttr);
	});

	AfterEach([this]()
	{
		if (TestActor)
		{
			TestActor->Destroy();
			TestActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should apply and expire temporary gameplay effect with attribute modifiers and granted tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBGameplayEffectSpec BerserkSpec;
		BerserkSpec.EffectTag = Tags.Effect_Buff_Berserk;
		BerserkSpec.EffectDisplayName = FText::FromString(TEXT("Berserk"));
		BerserkSpec.DurationType = ESBEffectDurationType::HasDuration;
		BerserkSpec.Duration = 3.0f;
		BerserkSpec.MaxStacks = 1;

		FSBGameplayEffectModifier Mod;
		Mod.AttributeTag = Tags.Attribute_Health;
		Mod.ModifierOp = ESBEffectModifierOp::Add;
		Mod.Magnitude = 50.0f;
		BerserkSpec.Modifiers.Add(Mod);
		BerserkSpec.GrantedTags.AddTag(Tags.State_Combat_Attacking);

		// 1. Aplica o efeito
		bool bApplied = EffectComp->ApplyGameplayEffectSpec(BerserkSpec);
		TestTrue("ApplyGameplayEffectSpec should return true", bApplied);
		TestTrue("Effect should be active", EffectComp->HasActiveGameplayEffect(Tags.Effect_Buff_Berserk));
		TestEqual("Health value with modifier should be 150", AttributeComp->GetAttributeValue(Tags.Attribute_Health), 150.0f);
		TestTrue("Granted tag should be present on state component", StateComp->HasTag(Tags.State_Combat_Attacking));

		// 2. Simula avanço de tempo e expiração (3.0s)
		EffectComp->TickComponent(3.0f, ELevelTick::LEVELTICK_All, nullptr);

		TestFalse("Effect should no longer be active", EffectComp->HasActiveGameplayEffect(Tags.Effect_Buff_Berserk));
		TestEqual("Health value should return to base 100", AttributeComp->GetAttributeValue(Tags.Attribute_Health), 100.0f);
		TestFalse("Granted tag should be removed", StateComp->HasTag(Tags.State_Combat_Attacking));
	});

	It("Should stack gameplay effect up to MaxStacks and scale magnitude", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBGameplayEffectSpec SpeedSpec;
		SpeedSpec.EffectTag = Tags.Effect_Buff_SpeedBoost;
		SpeedSpec.EffectDisplayName = FText::FromString(TEXT("Speed Boost"));
		SpeedSpec.DurationType = ESBEffectDurationType::HasDuration;
		SpeedSpec.Duration = 5.0f;
		SpeedSpec.MaxStacks = 3;

		FSBGameplayEffectModifier Mod;
		Mod.AttributeTag = Tags.Attribute_Health;
		Mod.ModifierOp = ESBEffectModifierOp::Add;
		Mod.Magnitude = 10.0f;
		SpeedSpec.Modifiers.Add(Mod);

		// 1. Stack 1
		EffectComp->ApplyGameplayEffectSpec(SpeedSpec);
		TestEqual("Stacks should be 1", EffectComp->GetActiveEffectStacks(Tags.Effect_Buff_SpeedBoost), 1);
		TestEqual("Health modifier (+10)", AttributeComp->GetAttributeValue(Tags.Attribute_Health), 110.0f);

		// 2. Stack 2
		EffectComp->ApplyGameplayEffectSpec(SpeedSpec);
		TestEqual("Stacks should be 2", EffectComp->GetActiveEffectStacks(Tags.Effect_Buff_SpeedBoost), 2);
		TestEqual("Health modifier (+20)", AttributeComp->GetAttributeValue(Tags.Attribute_Health), 120.0f);

		// 3. Stack 3
		EffectComp->ApplyGameplayEffectSpec(SpeedSpec);
		TestEqual("Stacks should be 3", EffectComp->GetActiveEffectStacks(Tags.Effect_Buff_SpeedBoost), 3);
		TestEqual("Health modifier (+30)", AttributeComp->GetAttributeValue(Tags.Attribute_Health), 130.0f);

		// 4. Stack 4 (Clamped no MaxStacks = 3)
		EffectComp->ApplyGameplayEffectSpec(SpeedSpec);
		TestEqual("Stacks should remain clamped at 3", EffectComp->GetActiveEffectStacks(Tags.Effect_Buff_SpeedBoost), 3);
	});

	It("Should block gameplay effect application when target has immunity tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// Concede imunidade a veneno
		StateComp->AddTag(Tags.State_Immunity_Poison);

		FSBGameplayEffectSpec PoisonSpec;
		PoisonSpec.EffectTag = Tags.Effect_Debuff_Poison;
		PoisonSpec.DurationType = ESBEffectDurationType::HasDuration;
		PoisonSpec.Duration = 5.0f;
		PoisonSpec.ImmunityTags.AddTag(Tags.State_Immunity_Poison);

		bool bApplied = EffectComp->ApplyGameplayEffectSpec(PoisonSpec);
		TestFalse("Application should be blocked by immunity tag", bApplied);
		TestFalse("Poison effect should not be active", EffectComp->HasActiveGameplayEffect(Tags.Effect_Debuff_Poison));
	});

	It("Should purge conflicting effects when specified in RemoveEffectsWithTags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// 1. Aplica veneno
		FSBGameplayEffectSpec PoisonSpec;
		PoisonSpec.EffectTag = Tags.Effect_Debuff_Poison;
		PoisonSpec.DurationType = ESBEffectDurationType::HasDuration;
		PoisonSpec.Duration = 10.0f;
		EffectComp->ApplyGameplayEffectSpec(PoisonSpec);
		TestTrue("Poison should be active", EffectComp->HasActiveGameplayEffect(Tags.Effect_Debuff_Poison));

		// 2. Aplica efeito de purificação/cura
		FSBGameplayEffectSpec CleanseSpec;
		CleanseSpec.EffectTag = Tags.Effect_Buff_Regeneration;
		CleanseSpec.DurationType = ESBEffectDurationType::Instant;
		CleanseSpec.RemoveEffectsWithTags.AddTag(Tags.Effect_Debuff_Poison);
		EffectComp->ApplyGameplayEffectSpec(CleanseSpec);

		TestFalse("Poison should be purged by Cleanse effect", EffectComp->HasActiveGameplayEffect(Tags.Effect_Debuff_Poison));
	});
}
