// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBLockOnComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBLockOnTestsSpec, "Sandbox.Combat.LockOnSystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* PlayerActor;
	AActor* TargetA;
	AActor* TargetB;
	USBLockOnComponent* LockOnComp;
	USBStateComponent* PlayerState;
	USBStateComponent* TargetAState;
	USBStateComponent* TargetBState;
END_DEFINE_SPEC(FSBLockOnTestsSpec)

void FSBLockOnTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 1. Player em (0, 0, 0) com rotação zero (olhando para +X)
		PlayerActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* PlayerRoot = NewObject<USceneComponent>(PlayerActor, TEXT("PlayerRoot"));
		PlayerActor->SetRootComponent(PlayerRoot);
		PlayerRoot->RegisterComponent();

		PlayerState = NewObject<USBStateComponent>(PlayerActor, TEXT("PlayerState"));
		PlayerState->RegisterComponent();
		PlayerActor->AddOwnedComponent(PlayerState);

		LockOnComp = NewObject<USBLockOnComponent>(PlayerActor, TEXT("LockOnComp"));
		LockOnComp->RegisterComponent();
		PlayerActor->AddOwnedComponent(LockOnComp);
		LockOnComp->Settings.bRequireLineOfSight = false; // Em testes unitários headless sem física complexa

		// 2. Alvo A em (500, 0, 0) (frontal)
		TargetA = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(500.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* TargetARoot = NewObject<USceneComponent>(TargetA, TEXT("TargetARoot"));
		TargetA->SetRootComponent(TargetARoot);
		TargetARoot->RegisterComponent();
		// Um AActor puro nao tem RootComponent no spawn, entao a localizacao passada a
		// SpawnActor nao e aplicada. Posicionar apos o root existir.
		TargetA->SetActorLocation(FVector(500.0f, 0.0f, 0.0f));

		TargetAState = NewObject<USBStateComponent>(TargetA, TEXT("TargetAState"));
		TargetAState->RegisterComponent();
		TargetA->AddOwnedComponent(TargetAState);

		// 3. Alvo B em (500, 300, 0) (à direita do Player)
		TargetB = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(500.0f, 300.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* TargetBRoot = NewObject<USceneComponent>(TargetB, TEXT("TargetBRoot"));
		TargetB->SetRootComponent(TargetBRoot);
		TargetBRoot->RegisterComponent();
		// Um AActor puro nao tem RootComponent no spawn, entao a localizacao passada a
		// SpawnActor nao e aplicada. Posicionar apos o root existir.
		TargetB->SetActorLocation(FVector(500.0f, 300.0f, 0.0f));

		TargetBState = NewObject<USBStateComponent>(TargetB, TEXT("TargetBState"));
		TargetBState->RegisterComponent();
		TargetB->AddOwnedComponent(TargetBState);

		ISBComponentInterface::Execute_OnInitialize(PlayerState);
		ISBComponentInterface::Execute_OnInitialize(LockOnComp);
		ISBComponentInterface::Execute_OnInitialize(TargetAState);
		ISBComponentInterface::Execute_OnInitialize(TargetBState);
	});

	AfterEach([this]()
	{
		if (PlayerActor)
		{
			PlayerActor->Destroy();
			PlayerActor = nullptr;
		}

		if (TargetA)
		{
			TargetA->Destroy();
			TargetA = nullptr;
		}

		if (TargetB)
		{
			TargetB->Destroy();
			TargetB = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should find and lock onto best target in frontal cone", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bLocked = LockOnComp->ToggleLockOn();
		TestTrue("ToggleLockOn should return true", bLocked);
		TestTrue("IsLockedOn should be true", LockOnComp->IsLockedOn());
		TestEqual("Target should be Target A (frontal / closest)", LockOnComp->GetCurrentTarget(), TargetA);
		TestTrue("Player should have LockedOn tag", PlayerState->HasTag(Tags.State_Combat_LockedOn));
		TestTrue("Target A should have Target tag", TargetAState->HasTag(Tags.State_Combat_Target));
	});

	It("Should switch target horizontally to the right", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// Trava inicial no Target A
		LockOnComp->LockOnTarget(TargetA);
		TestEqual("Current target is Target A", LockOnComp->GetCurrentTarget(), TargetA);

		// Alterna para a direita (Target B está em Y=300)
		bool bSwitched = LockOnComp->SwitchTarget(ESBLockOnSwitchDirection::Right);
		TestTrue("SwitchTarget Right should succeed", bSwitched);
		TestEqual("Current target should now be Target B", LockOnComp->GetCurrentTarget(), TargetB);
		TestFalse("Target A should no longer have Target tag", TargetAState->HasTag(Tags.State_Combat_Target));
		TestTrue("Target B should now have Target tag", TargetBState->HasTag(Tags.State_Combat_Target));
	});

	It("Should calculate desired look at rotation facing locked target", [this]()
	{
		LockOnComp->LockOnTarget(TargetA);

		FRotator DesiredRot = LockOnComp->GetDesiredRotationToTarget();
		TestNearlyEqual("Yaw should face +X directly (0 deg)", (float)DesiredRot.Yaw, 0.0f, 1.0f);
		TestNearlyEqual("Pitch should be flat (0 deg)", (float)DesiredRot.Pitch, 0.0f, 1.0f);
	});

	It("Should automatically break lock when distance exceeds BreakDistance", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		LockOnComp->LockOnTarget(TargetA);
		TestTrue("Locked on Target A", LockOnComp->IsLockedOn());

		// Move Target A para além de BreakDistance (2500)
		TargetA->SetActorLocation(FVector(3000.0f, 0.0f, 0.0f));

		LockOnComp->TickComponent(0.1f, ELevelTick::LEVELTICK_All, nullptr);

		TestFalse("Lock should be broken", LockOnComp->IsLockedOn());
		TestNull("Current target should be null", LockOnComp->GetCurrentTarget());
		TestFalse("Player should lose LockedOn tag", PlayerState->HasTag(Tags.State_Combat_LockedOn));
	});
}
