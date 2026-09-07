// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBMechComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBMechTestsSpec, "Sandbox.Character.MechAndExosuit", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* MechActor;
	AActor* PilotActor;
	USBMechComponent* MechComp;
	USBStateComponent* MechStateComp;
	USBStateComponent* PilotStateComp;
END_DEFINE_SPEC(FSBMechTestsSpec)

void FSBMechTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		MechActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* MechRoot = NewObject<USceneComponent>(MechActor, TEXT("MechRoot"));
		MechActor->SetRootComponent(MechRoot);
		MechRoot->RegisterComponent();

		MechStateComp = NewObject<USBStateComponent>(MechActor, TEXT("MechStateComp"));
		MechActor->AddOwnedComponent(MechStateComp);

		MechComp = NewObject<USBMechComponent>(MechActor, TEXT("MechComp"));
		MechActor->AddOwnedComponent(MechComp);

		PilotActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* PilotRoot = NewObject<USceneComponent>(PilotActor, TEXT("PilotRoot"));
		PilotActor->SetRootComponent(PilotRoot);
		PilotRoot->RegisterComponent();

		PilotStateComp = NewObject<USBStateComponent>(PilotActor, TEXT("PilotStateComp"));
		PilotActor->AddOwnedComponent(PilotStateComp);

		ISBComponentInterface::Execute_OnInitialize(MechStateComp);
		ISBComponentInterface::Execute_OnInitialize(MechComp);
		ISBComponentInterface::Execute_OnInitialize(PilotStateComp);
	});

	AfterEach([this]()
	{
		if (PilotActor)
		{
			PilotActor->Destroy();
			PilotActor = nullptr;
		}

		if (MechActor)
		{
			MechActor->Destroy();
			MechActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should enter mech cockpit, power on systems, grant mech tags, and set state to Idle", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bPilotFired = false;
		MechComp->OnMechPilotChanged.AddLambda([&bPilotFired](AActor* Pilot)
		{
			bPilotFired = true;
		});

		bool bEntered = MechComp->EnterMech(PilotActor);

		TestTrue("Enter succeeded", bEntered);
		TestTrue("Is pilot", MechComp->IsPilot(PilotActor));
		TestEqual("State is Idle", (int32)MechComp->GetMechState(), (int32)ESBMechState::Idle);
		TestTrue("Mech has Mech tag", MechStateComp->HasTag(Tags.State_Vehicle_Mech));
		TestTrue("Pilot has Mech tag", PilotStateComp->HasTag(Tags.State_Movement_Mech));
		TestTrue("Pilot delegate fired", bPilotFired);
	});

	It("Should apply move input, advance bipedal stride, set Walking state, and grant Walking tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		MechComp->EnterMech(PilotActor);
		MechComp->SetMoveInput(FVector(1.0f, 0.0f, 0.0f));
		MechComp->UpdateMechPhysics(1.0f);

		TestTrue("Speed increased", MechComp->GetOperationalData().CurrentSpeed > 0.0f);
		TestEqual("State is Walking", (int32)MechComp->GetMechState(), (int32)ESBMechState::Walking);
		TestTrue("Pilot has Walking tag", PilotStateComp->HasTag(Tags.State_Movement_Mech_Walking));
	});

	It("Should activate jump jets, consume fuel, generate core heat, and grant JumpJets tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		MechComp->EnterMech(PilotActor);
		MechComp->ActivateJumpJets(true);
		MechComp->UpdateMechPhysics(1.0f);

		TestTrue("Fuel drained", MechComp->GetOperationalData().JumpJetFuel < 100.0f);
		TestTrue("Heat increased", MechComp->GetOperationalData().CoreHeat > 0.0f);
		TestEqual("State is JumpJets", (int32)MechComp->GetMechState(), (int32)ESBMechState::JumpJets);
		TestTrue("Pilot has JumpJets tag", PilotStateComp->HasTag(Tags.State_Movement_Mech_JumpJets));
	});

	It("Should trigger overheat shutdown on excess heat, lock movement, exit mech, and clear all tags cleanly", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		MechComp->EnterMech(PilotActor);
		MechComp->ActivateJumpJets(true);

		// Heat up until overheat
		for (int32 i = 0; i < 5; ++i)
		{
			MechComp->UpdateMechPhysics(1.0f);
		}

		TestTrue("Is Overheated", MechComp->IsOverheated());
		TestEqual("State is Overheated", (int32)MechComp->GetMechState(), (int32)ESBMechState::Overheated);
		TestTrue("Pilot has Overheated tag", PilotStateComp->HasTag(Tags.State_Movement_Mech_Overheated));

		bool bExited = MechComp->ExitMech(PilotActor);

		TestTrue("Exit succeeded", bExited);
		TestFalse("Pilot no longer pilot", MechComp->IsPilot(PilotActor));
		TestEqual("State is PoweredOff", (int32)MechComp->GetMechState(), (int32)ESBMechState::PoweredOff);
		TestFalse("Pilot Mech tag removed", PilotStateComp->HasTag(Tags.State_Movement_Mech));
		TestFalse("Pilot Walking tag removed", PilotStateComp->HasTag(Tags.State_Movement_Mech_Walking));
		TestFalse("Pilot JumpJets tag removed", PilotStateComp->HasTag(Tags.State_Movement_Mech_JumpJets));
		TestFalse("Pilot Overheated tag removed", PilotStateComp->HasTag(Tags.State_Movement_Mech_Overheated));
	});
}
