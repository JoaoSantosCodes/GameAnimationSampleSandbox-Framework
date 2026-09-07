// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBPoiseComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBPoiseTestsSpec, "Sandbox.Combat.PoiseAndSuperArmor", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TargetActor;
	USBPoiseComponent* PoiseComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBPoiseTestsSpec)

void FSBPoiseTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn Target em (0, 0, 0) com rotação zero (Forward = +X, Right = +Y)
		TargetActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(TargetActor, TEXT("Root"));
		TargetActor->SetRootComponent(Root);
		Root->RegisterComponent();

		StateComp = NewObject<USBStateComponent>(TargetActor, TEXT("StateComp"));
		StateComp->RegisterComponent();
		TargetActor->AddOwnedComponent(StateComp);

		PoiseComp = NewObject<USBPoiseComponent>(TargetActor, TEXT("PoiseComp"));
		PoiseComp->RegisterComponent();
		TargetActor->AddOwnedComponent(PoiseComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(PoiseComp);
	});

	AfterEach([this]()
	{
		if (TargetActor)
		{
			TargetActor->Destroy();
			TargetActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should calculate directional hit reactions from front, back, left, and right", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// Frontal
		FSBHitReactionResult ResultFront;
		PoiseComp->ReceivePoiseDamage(10.0f, FVector(100.0f, 0.0f, 0.0f), nullptr, ResultFront);
		TestEqual("Front direction detected", (int32)ResultFront.Direction, (int32)ESBHitReactionDirection::Front);
		TestEqual("Front tag applied", ResultFront.ReactionTag, Tags.Combat_Reaction_Front);

		// Traseiro
		FSBHitReactionResult ResultBack;
		PoiseComp->ReceivePoiseDamage(10.0f, FVector(-100.0f, 0.0f, 0.0f), nullptr, ResultBack);
		TestEqual("Back direction detected", (int32)ResultBack.Direction, (int32)ESBHitReactionDirection::Back);
		TestEqual("Back tag applied", ResultBack.ReactionTag, Tags.Combat_Reaction_Back);

		// Direita (+Y)
		FSBHitReactionResult ResultRight;
		PoiseComp->ReceivePoiseDamage(10.0f, FVector(0.0f, 100.0f, 0.0f), nullptr, ResultRight);
		TestEqual("Right direction detected", (int32)ResultRight.Direction, (int32)ESBHitReactionDirection::Right);
		TestEqual("Right tag applied", ResultRight.ReactionTag, Tags.Combat_Reaction_Right);

		// Esquerda (-Y)
		FSBHitReactionResult ResultLeft;
		PoiseComp->ReceivePoiseDamage(10.0f, FVector(0.0f, -100.0f, 0.0f), nullptr, ResultLeft);
		TestEqual("Left direction detected", (int32)ResultLeft.Direction, (int32)ESBHitReactionDirection::Left);
		TestEqual("Left tag applied", ResultLeft.ReactionTag, Tags.Combat_Reaction_Left);
	});

	It("Should break poise and apply stagger when poise health is depleted", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBHitReactionResult Result;
		PoiseComp->ReceivePoiseDamage(100.0f, FVector(100.0f, 0.0f, 0.0f), nullptr, Result);

		TestTrue("Poise should be broken", Result.bPoiseBroken);
		TestEqual("Current poise should be 0", PoiseComp->GetCurrentPoise(), 0.0f);
		TestTrue("Poise broken flag is true", PoiseComp->IsPoiseBroken());
		TestTrue("State component received PoiseBroken tag", StateComp->HasTag(Tags.State_Combat_PoiseBroken));
		TestEqual("High damage poise break triggers Knockdown intensity", (int32)Result.Intensity, (int32)ESBHitReactionIntensity::Knockdown);
	});

	It("Should absorb hit reaction when super armor is active", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		PoiseComp->SetSuperArmor(true);
		TestTrue("Super armor tag applied", StateComp->HasTag(Tags.State_Combat_SuperArmor));

		FSBHitReactionResult Result;
		PoiseComp->ReceivePoiseDamage(50.0f, FVector(100.0f, 0.0f, 0.0f), nullptr, Result);

		TestTrue("Damage absorbed by super armor", Result.bAbsorbedBySuperArmor);
		TestEqual("Reaction intensity is None during super armor", (int32)Result.Intensity, (int32)ESBHitReactionIntensity::None);
		TestFalse("Poise not broken", Result.bPoiseBroken);
	});

	It("Should regenerate poise after delay", [this]()
	{
		FSBHitReactionResult Result;
		PoiseComp->ReceivePoiseDamage(40.0f, FVector(100.0f, 0.0f, 0.0f), nullptr, Result);
		TestEqual("Poise reduced to 60", PoiseComp->GetCurrentPoise(), 60.0f);

		// Simula 2 segundos (delay de 3s ainda não expirou)
		PoiseComp->TickComponent(2.0f, ELevelTick::LEVELTICK_All, nullptr);
		TestEqual("Poise should not regenerate during delay", PoiseComp->GetCurrentPoise(), 60.0f);

		// Simula mais 2 segundos (delay de 3s expirou, 1s de regeneração a 20/s)
		PoiseComp->TickComponent(2.0f, ELevelTick::LEVELTICK_All, nullptr);
		TestEqual("Poise should regenerate to 80", PoiseComp->GetCurrentPoise(), 80.0f);
	});
}
