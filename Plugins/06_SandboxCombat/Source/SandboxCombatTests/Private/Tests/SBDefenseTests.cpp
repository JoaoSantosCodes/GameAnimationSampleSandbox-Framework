// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/SBDefenseComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBDefenseTestsSpec, "Sandbox.Combat.DefenseAndParry", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* DefenderActor;
	AActor* AttackerActor;
	USBAttributeComponent* DefenderAttributes;
	USBStateComponent* DefenderState;
	USBDefenseComponent* DefenderDefense;
	USBStateComponent* AttackerState;
END_DEFINE_SPEC(FSBDefenseTestsSpec)

void FSBDefenseTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		DefenderActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		// Um AActor puro nao tem RootComponent: sem ele, SetActorLocation, SetActorTransform
		// e TeleportTo falham em silencio e o ator fica preso na origem.
		USceneComponent* DefenderActorRoot = NewObject<USceneComponent>(DefenderActor, TEXT("DefenderActorRoot"));
		DefenderActor->SetRootComponent(DefenderActorRoot);
		DefenderActorRoot->RegisterComponent();
		AttackerActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		USceneComponent* AttackerActorRoot = NewObject<USceneComponent>(AttackerActor, TEXT("AttackerActorRoot"));
		AttackerActor->SetRootComponent(AttackerActorRoot);
		AttackerActorRoot->RegisterComponent();
		AttackerActor->SetActorLocation(FVector(100.0f, 0.0f, 0.0f));

		DefenderAttributes = NewObject<USBAttributeComponent>(DefenderActor, TEXT("DefenderAttributes"));
		DefenderAttributes->RegisterComponent();
		DefenderActor->AddOwnedComponent(DefenderAttributes);

		DefenderState = NewObject<USBStateComponent>(DefenderActor, TEXT("DefenderState"));
		DefenderState->RegisterComponent();
		DefenderActor->AddOwnedComponent(DefenderState);

		DefenderDefense = NewObject<USBDefenseComponent>(DefenderActor, TEXT("DefenderDefense"));
		DefenderDefense->RegisterComponent();
		DefenderActor->AddOwnedComponent(DefenderDefense);

		AttackerState = NewObject<USBStateComponent>(AttackerActor, TEXT("AttackerState"));
		AttackerState->RegisterComponent();
		AttackerActor->AddOwnedComponent(AttackerState);

		ISBComponentInterface::Execute_OnInitialize(DefenderAttributes);
		ISBComponentInterface::Execute_OnInitialize(DefenderState);
		ISBComponentInterface::Execute_OnInitialize(DefenderDefense);
		ISBComponentInterface::Execute_OnInitialize(AttackerState);

		// Registra Estamina base = 100
		FSBAttribute StaminaAttr;
		StaminaAttr.BaseValue = 100.0f;
		StaminaAttr.CurrentValue = 100.0f;
		StaminaAttr.MaxValue = 100.0f;
		StaminaAttr.MinValue = 0.0f;
		DefenderAttributes->RegisterAttribute(FSBGameplayTags::Get().Attribute_Stamina, StaminaAttr);
	});

	AfterEach([this]()
	{
		if (DefenderActor)
		{
			DefenderActor->Destroy();
			DefenderActor = nullptr;
		}

		if (AttackerActor)
		{
			AttackerActor->Destroy();
			AttackerActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should mitigate damage and consume stamina during regular block", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		DefenderDefense->StartBlocking();
		TestTrue("Defender should be in blocking state", DefenderDefense->IsBlocking());

		// Simula 0.3s (Parry window de 0.25s expira)
		DefenderDefense->TickComponent(0.3f, ELevelTick::LEVELTICK_All, nullptr);
		TestFalse("Parry window should be closed", DefenderDefense->IsParryActive());

		float MitigatedDamage = 0.0f;
		ESBBlockResult Result = DefenderDefense->ProcessIncomingDamage(100.0f, AttackerActor, MitigatedDamage);

		TestEqual("Result should be Blocked", (int32)Result, (int32)ESBBlockResult::Blocked);
		TestEqual("Damage should be mitigated by 60% (40 damage)", MitigatedDamage, 40.0f);
		TestEqual("Stamina should decrease by 15 (85 stamina remaining)", DefenderAttributes->GetAttributeValue(Tags.Attribute_Stamina), 85.0f);
	});

	It("Should perform perfect parry deflection within parry window", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		DefenderDefense->StartBlocking();
		TestTrue("Parry should be active immediately after block start", DefenderDefense->IsParryActive());

		float MitigatedDamage = 0.0f;
		ESBBlockResult Result = DefenderDefense->ProcessIncomingDamage(100.0f, AttackerActor, MitigatedDamage);

		TestEqual("Result should be Parried", (int32)Result, (int32)ESBBlockResult::Parried);
		TestEqual("Damage should be completely negated (0 damage)", MitigatedDamage, 0.0f);
		TestEqual("Stamina should not be consumed (100 remaining)", DefenderAttributes->GetAttributeValue(Tags.Attribute_Stamina), 100.0f);
		TestTrue("Attacker should receive Staggered state tag", AttackerState->HasTag(Tags.State_Combat_Staggered));
		TestTrue("Defender should have CounterAttackReady tag", DefenderState->HasTag(Tags.State_Combat_CounterAttackReady));

		float CounterMultiplier = DefenderDefense->ConsumeCounterAttack();
		TestEqual("Counter attack multiplier should be 2.0x", CounterMultiplier, 2.0f);
		TestFalse("CounterAttackReady tag should be removed after consumption", DefenderState->HasTag(Tags.State_Combat_CounterAttackReady));
	});

	It("Should break guard when defender stamina is depleted", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// Define estamina baixa (5.0f < 15.0f custo de bloco)
		DefenderAttributes->SetAttributeBaseValue(Tags.Attribute_Stamina, 5.0f);

		DefenderDefense->StartBlocking();
		DefenderDefense->TickComponent(0.3f, ELevelTick::LEVELTICK_All, nullptr);

		float MitigatedDamage = 0.0f;
		ESBBlockResult Result = DefenderDefense->ProcessIncomingDamage(100.0f, AttackerActor, MitigatedDamage);

		TestEqual("Result should be GuardBroken", (int32)Result, (int32)ESBBlockResult::GuardBroken);
		TestEqual("Full unmitigated damage passed through", MitigatedDamage, 100.0f);
		TestEqual("Stamina should be depleted to 0", DefenderAttributes->GetAttributeValue(Tags.Attribute_Stamina), 0.0f);
		TestTrue("Defender should receive GuardBroken tag", DefenderState->HasTag(Tags.State_Combat_GuardBroken));
		TestFalse("Defender should no longer be blocking", DefenderDefense->IsBlocking());
	});

	It("Should expire counter attack window after timeout", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		DefenderDefense->StartBlocking();
		float MitigatedDamage = 0.0f;
		DefenderDefense->ProcessIncomingDamage(50.0f, AttackerActor, MitigatedDamage);

		TestTrue("Counter attack window is active", DefenderDefense->IsCounterAttackReady());

		// Simula 1.5s (janela de 1.0s expira)
		DefenderDefense->TickComponent(1.5f, ELevelTick::LEVELTICK_All, nullptr);

		TestFalse("Counter attack window should be expired", DefenderDefense->IsCounterAttackReady());
		TestFalse("CounterAttackReady tag should be removed", DefenderState->HasTag(Tags.State_Combat_CounterAttackReady));
	});
}
