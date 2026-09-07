// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBStealthComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBStealthTestsSpec, "Sandbox.Combat.StealthAndPerception", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* StealthActor;
	USBStealthComponent* StealthComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBStealthTestsSpec)

void FSBStealthTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		StealthActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(StealthActor, TEXT("Root"));
		StealthActor->SetRootComponent(Root);
		Root->RegisterComponent();

		StateComp = NewObject<USBStateComponent>(StealthActor, TEXT("StateComp"));
		StateComp->RegisterComponent();
		StealthActor->AddOwnedComponent(StateComp);

		StealthComp = NewObject<USBStealthComponent>(StealthActor, TEXT("StealthComp"));
		StealthComp->RegisterComponent();
		StealthActor->AddOwnedComponent(StealthComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(StealthComp);
	});

	AfterEach([this]()
	{
		if (StealthActor)
		{
			StealthActor->Destroy();
			StealthActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should emit noise and record noise event in history", [this]()
	{
		FSBNoiseEvent Result = StealthComp->EmitNoise(750.0f, 1.8f);

		TestEqual("Radius recorded", Result.Radius, 750.0f);
		TestEqual("Loudness recorded", Result.Loudness, 1.8f);
		TestEqual("History count is 1", StealthComp->GetNoiseHistoryCount(), 1);
	});

	It("Should accumulate alert and transition from Hidden to Suspicious to Detected", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		TestEqual("Initial state is Hidden", (int32)StealthComp->GetStealthState(), (int32)ESBStealthState::Hidden);
		TestTrue("Has Hidden state tag", StateComp->HasTag(Tags.State_Combat_Stealth_Hidden));

		// Base exposure 1.0, AlertBuildRate = 2.0. In 0.25s alert becomes 0.5f (Suspicious)
		ESBStealthState State1 = StealthComp->UpdateDetection(1.0f, 0.25f);
		TestEqual("State transitioned to Suspicious", (int32)State1, (int32)ESBStealthState::Suspicious);
		TestTrue("Has Suspicious tag", StateComp->HasTag(Tags.State_Combat_Stealth_Suspicious));
		TestFalse("No longer has Hidden tag", StateComp->HasTag(Tags.State_Combat_Stealth_Hidden));

		// Another 0.35s pushes alert to 1.0f (Detected)
		ESBStealthState State2 = StealthComp->UpdateDetection(1.0f, 0.35f);
		TestEqual("State transitioned to Detected", (int32)State2, (int32)ESBStealthState::Detected);
		TestTrue("Has Detected tag", StateComp->HasTag(Tags.State_Combat_Stealth_Detected));
		TestFalse("No longer has Suspicious tag", StateComp->HasTag(Tags.State_Combat_Stealth_Suspicious));
	});

	It("Should attenuate visibility when crouching and in shadows", [this]()
	{
		TestNearlyEqual("Base visibility is 1.0", StealthComp->GetEffectiveVisibility(), 1.0f, 0.001f);

		StealthComp->SetCrouched(true);
		TestNearlyEqual("Crouch visibility is 0.5", StealthComp->GetEffectiveVisibility(), 0.5f, 0.001f);

		StealthComp->SetInShadows(true);
		TestNearlyEqual("Crouch + shadow visibility is 0.2", StealthComp->GetEffectiveVisibility(), 0.2f, 0.001f);
	});

	It("Should decay alert and return to Hidden state when unobserved", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// Accumula alert para Suspicious
		StealthComp->UpdateDetection(1.0f, 0.3f);
		TestEqual("State is Suspicious", (int32)StealthComp->GetStealthState(), (int32)ESBStealthState::Suspicious);

		// Unobserved: exposure = 0.0f. AlertDecayRate = 1.0f. Decai 1.0s -> chega a 0.0f
		ESBStealthState RestoredState = StealthComp->UpdateDetection(0.0f, 1.0f);
		TestEqual("Restored to Hidden state", (int32)RestoredState, (int32)ESBStealthState::Hidden);
		TestEqual("Alert percent is 0", StealthComp->GetAlertPercent(), 0.0f);
		TestTrue("Has Hidden tag restored", StateComp->HasTag(Tags.State_Combat_Stealth_Hidden));
		TestFalse("Suspicious tag removed", StateComp->HasTag(Tags.State_Combat_Stealth_Suspicious));
	});
}
