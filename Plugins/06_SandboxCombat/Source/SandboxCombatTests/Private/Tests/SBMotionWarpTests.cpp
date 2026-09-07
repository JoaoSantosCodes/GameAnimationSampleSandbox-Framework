// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBMotionWarpComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBMotionWarpTestsSpec, "Sandbox.Combat.MotionWarping", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* AttackerActor;
	AActor* TargetActor;
	USBMotionWarpComponent* MotionWarpComp;
	USBStateComponent* AttackerState;
END_DEFINE_SPEC(FSBMotionWarpTestsSpec)

void FSBMotionWarpTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 1. Atacante em (0, 0, 0)
		AttackerActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* AttackerRoot = NewObject<USceneComponent>(AttackerActor, TEXT("AttackerRoot"));
		AttackerActor->SetRootComponent(AttackerRoot);
		AttackerRoot->RegisterComponent();

		AttackerState = NewObject<USBStateComponent>(AttackerActor, TEXT("AttackerState"));
		AttackerState->RegisterComponent();
		AttackerActor->AddOwnedComponent(AttackerState);

		MotionWarpComp = NewObject<USBMotionWarpComponent>(AttackerActor, TEXT("MotionWarpComp"));
		MotionWarpComp->RegisterComponent();
		AttackerActor->AddOwnedComponent(MotionWarpComp);

		// 2. Alvo em (300, 0, 0)
		TargetActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(300.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* TargetRoot = NewObject<USceneComponent>(TargetActor, TEXT("TargetRoot"));
		TargetActor->SetRootComponent(TargetRoot);
		TargetRoot->RegisterComponent();
		// Obrigatório: um AActor puro não possui RootComponent no momento do spawn, então a
		// localização passada a SpawnActor não é aplicada. Sem esta linha o alvo permanece na
		// origem e GetActorLocation() retorna (0,0,0).
		TargetActor->SetActorLocation(FVector(300.0f, 0.0f, 0.0f));

		ISBComponentInterface::Execute_OnInitialize(AttackerState);
		ISBComponentInterface::Execute_OnInitialize(MotionWarpComp);
	});

	AfterEach([this]()
	{
		if (AttackerActor)
		{
			AttackerActor->Destroy();
			AttackerActor = nullptr;
		}

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

	It("Should initialize motion warp and apply state tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// Alvo a 300 unidades com offset de 100 -> ponto calculado deve ser (200, 0, 0)
		MotionWarpComp->StartMotionWarpToActor(TargetActor, 0.3f, 100.0f);

		TestTrue("Should be in Warping state", MotionWarpComp->IsWarping());
		TestEqual("State should be Warping", (int32)MotionWarpComp->GetWarpState(), (int32)ESBMotionWarpState::Warping);
		TestTrue("Attacker should have MotionWarping tag", AttackerState->HasTag(Tags.State_Combat_MotionWarping));
		TestNearlyEqual("Calculated X target should be 200", (float)MotionWarpComp->GetCalculatedTargetLocation().X, 200.0f, 1.0f);
	});

	It("Should smoothly translate towards target over duration", [this]()
	{
		MotionWarpComp->StartMotionWarpToActor(TargetActor, 0.3f, 100.0f);

		// Simula 0.15s (50% do warp de 0.3s)
		MotionWarpComp->TickComponent(0.15f, ELevelTick::LEVELTICK_All, nullptr);

		TestTrue("Warping should still be active at 50%", MotionWarpComp->IsWarping());
		// X inicial era 0, alvo calculado é 200, logo em 50% deve estar em ~100
		TestNearlyEqual("Midway location X should be ~100", (float)AttackerActor->GetActorLocation().X, 100.0f, 10.0f);
	});

	It("Should complete motion warp and clear state tags on duration finish", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		MotionWarpComp->StartMotionWarpToActor(TargetActor, 0.3f, 100.0f);

		// Simula 0.35s (ultrapassando 0.3s de duração)
		MotionWarpComp->TickComponent(0.35f, ELevelTick::LEVELTICK_All, nullptr);

		TestFalse("Warping should no longer be active", MotionWarpComp->IsWarping());
		TestEqual("State should be Completed", (int32)MotionWarpComp->GetWarpState(), (int32)ESBMotionWarpState::Completed);
		TestFalse("MotionWarping tag should be removed", AttackerState->HasTag(Tags.State_Combat_MotionWarping));
		TestNearlyEqual("Final location matches calculated target (200)", (float)AttackerActor->GetActorLocation().X, 200.0f, 1.0f);
	});

	It("Should abort motion warp cleanly", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		MotionWarpComp->StartMotionWarpToActor(TargetActor, 0.3f, 100.0f);
		TestTrue("Warp active", MotionWarpComp->IsWarping());

		MotionWarpComp->StopMotionWarp(true);

		TestFalse("Warp inactive", MotionWarpComp->IsWarping());
		TestEqual("State should be Aborted", (int32)MotionWarpComp->GetWarpState(), (int32)ESBMotionWarpState::Aborted);
		TestFalse("MotionWarping tag removed", AttackerState->HasTag(Tags.State_Combat_MotionWarping));
	});
}
