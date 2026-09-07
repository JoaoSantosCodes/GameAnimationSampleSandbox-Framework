// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBSwimComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBSwimTestsSpec, "Sandbox.Character.SwimmingAndBuoyancy", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBSwimComponent* SwimComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBSwimTestsSpec)

void FSBSwimTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CharacterActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(CharacterActor, TEXT("Root"));
		CharacterActor->SetRootComponent(Root);
		Root->RegisterComponent();

		StateComp = NewObject<USBStateComponent>(CharacterActor, TEXT("StateComp"));
		CharacterActor->AddOwnedComponent(StateComp);

		SwimComp = NewObject<USBSwimComponent>(CharacterActor, TEXT("SwimComp"));
		CharacterActor->AddOwnedComponent(SwimComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(SwimComp);
	});

	AfterEach([this]()
	{
		if (CharacterActor)
		{
			CharacterActor->Destroy();
			CharacterActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should enter water, start surface swimming, and apply swimming state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bStateChangedFired = false;
		ESBSwimState FiredState = ESBSwimState::None;
		SwimComp->OnSwimStateChanged.AddLambda([&bStateChangedFired, &FiredState](ESBSwimState NewState)
		{
			bStateChangedFired = true;
			FiredState = NewState;
		});

		SwimComp->EnterWater(100.0f);

		TestTrue("Is swimming", SwimComp->IsSwimming());
		TestEqual("State is SurfaceSwimming", (int32)SwimComp->GetSwimState(), (int32)ESBSwimState::SurfaceSwimming);
		TestTrue("Has Swimming tag", StateComp->HasTag(Tags.State_Movement_Swimming));
		TestTrue("Has Surface tag", StateComp->HasTag(Tags.State_Movement_Swimming_Surface));
		TestFalse("Does not have Diving tag", StateComp->HasTag(Tags.State_Movement_Swimming_Diving));
		TestTrue("Delegate fired", bStateChangedFired);
		TestEqual("Delegate state is SurfaceSwimming", (int32)FiredState, (int32)ESBSwimState::SurfaceSwimming);
	});

	It("Should transition to diving and update diving state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SwimComp->EnterWater(100.0f);
		SwimComp->StartDiving();

		TestTrue("Is diving", SwimComp->IsDiving());
		TestEqual("State is Diving", (int32)SwimComp->GetSwimState(), (int32)ESBSwimState::Diving);
		TestTrue("Has Swimming tag", StateComp->HasTag(Tags.State_Movement_Swimming));
		TestTrue("Has Diving tag", StateComp->HasTag(Tags.State_Movement_Swimming_Diving));
		TestFalse("Does not have Surface tag", StateComp->HasTag(Tags.State_Movement_Swimming_Surface));
	});

	It("Should deplete oxygen while diving and trigger drowning state when depleted", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SwimComp->EnterWater(100.0f);
		SwimComp->StartDiving();

		bool bDrowningFired = false;
		SwimComp->OnDrowningStarted.AddLambda([&bDrowningFired]()
		{
			bDrowningFired = true;
		});

		// Deplete all 100 oxygen (at 5 oxygen/sec, 20 seconds needed)
		SwimComp->ConsumeOxygen(20.0f);

		TestEqual("Oxygen is zero", SwimComp->GetOxygenData().CurrentOxygen, 0.0f);
		TestTrue("Is drowning flag set", SwimComp->GetOxygenData().bIsDrowning);
		TestTrue("Has Drowning tag", StateComp->HasTag(Tags.State_Status_Drowning));
		TestTrue("Drowning delegate fired", bDrowningFired);
	});

	It("Should surface from dive, recover oxygen, and clear aquatic tags on exit", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SwimComp->EnterWater(100.0f);
		SwimComp->StartDiving();
		SwimComp->ConsumeOxygen(20.0f);

		TestTrue("Is drowning before surface", SwimComp->GetOxygenData().bIsDrowning);

		SwimComp->SurfaceFromDive();
		TestEqual("State is SurfaceSwimming", (int32)SwimComp->GetSwimState(), (int32)ESBSwimState::SurfaceSwimming);

		SwimComp->RecoverOxygen(2.0f); // Recovers 40 oxygen (at 20/s)
		TestTrue("Oxygen recovered", SwimComp->GetOxygenData().CurrentOxygen > 0.0f);
		TestFalse("Not drowning", SwimComp->GetOxygenData().bIsDrowning);
		TestFalse("Drowning tag removed", StateComp->HasTag(Tags.State_Status_Drowning));

		SwimComp->ExitWater();
		TestFalse("Not swimming after exit", SwimComp->IsSwimming());
		TestEqual("State is None", (int32)SwimComp->GetSwimState(), (int32)ESBSwimState::None);
		TestFalse("Swimming tag removed", StateComp->HasTag(Tags.State_Movement_Swimming));
		TestFalse("Surface tag removed", StateComp->HasTag(Tags.State_Movement_Swimming_Surface));
		TestFalse("Diving tag removed", StateComp->HasTag(Tags.State_Movement_Swimming_Diving));
	});
}
