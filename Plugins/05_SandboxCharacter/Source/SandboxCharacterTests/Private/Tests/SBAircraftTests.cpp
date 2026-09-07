// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBAircraftComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBAircraftTestsSpec, "Sandbox.Character.AircraftAndFlight", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* AircraftActor;
	AActor* PilotActor;
	USBAircraftComponent* AircraftComp;
	USBStateComponent* AircraftStateComp;
	USBStateComponent* PilotStateComp;
END_DEFINE_SPEC(FSBAircraftTestsSpec)

void FSBAircraftTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AircraftActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* AircraftRoot = NewObject<USceneComponent>(AircraftActor, TEXT("AircraftRoot"));
		AircraftActor->SetRootComponent(AircraftRoot);
		AircraftRoot->RegisterComponent();

		AircraftStateComp = NewObject<USBStateComponent>(AircraftActor, TEXT("AircraftStateComp"));
		AircraftActor->AddOwnedComponent(AircraftStateComp);

		AircraftComp = NewObject<USBAircraftComponent>(AircraftActor, TEXT("AircraftComp"));
		AircraftActor->AddOwnedComponent(AircraftComp);

		PilotActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* PilotRoot = NewObject<USceneComponent>(PilotActor, TEXT("PilotRoot"));
		PilotActor->SetRootComponent(PilotRoot);
		PilotRoot->RegisterComponent();

		PilotStateComp = NewObject<USBStateComponent>(PilotActor, TEXT("PilotStateComp"));
		PilotActor->AddOwnedComponent(PilotStateComp);

		ISBComponentInterface::Execute_OnInitialize(AircraftStateComp);
		ISBComponentInterface::Execute_OnInitialize(AircraftComp);
		ISBComponentInterface::Execute_OnInitialize(PilotStateComp);
	});

	AfterEach([this]()
	{
		if (PilotActor)
		{
			PilotActor->Destroy();
			PilotActor = nullptr;
		}

		if (AircraftActor)
		{
			AircraftActor->Destroy();
			AircraftActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should enter aircraft as pilot, grant flying tags, and set state to Taxiing", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bPilotFired = false;
		AircraftComp->OnAircraftPilotChanged.AddLambda([&bPilotFired](AActor* Pilot)
		{
			bPilotFired = true;
		});

		bool bEntered = AircraftComp->EnterAircraft(PilotActor);

		TestTrue("Enter succeeded", bEntered);
		TestTrue("Is pilot", AircraftComp->IsPilot(PilotActor));
		TestEqual("State is Taxiing", (int32)AircraftComp->GetFlightState(), (int32)ESBFlightState::Taxiing);
		TestTrue("Aircraft has Aircraft tag", AircraftStateComp->HasTag(Tags.State_Vehicle_Aircraft));
		TestTrue("Pilot has Flying tag", PilotStateComp->HasTag(Tags.State_Movement_Flying));
		TestTrue("Pilot delegate fired", bPilotFired);
	});

	It("Should accelerate throttle past stall speed, become airborne, and grant Airborne tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AircraftComp->EnterAircraft(PilotActor);
		AircraftComp->SetThrottleInput(1.0f);
		AircraftComp->UpdateFlightPhysics(1.0f, FVector::ZeroVector);

		TestTrue("Speed above stall speed", AircraftComp->GetFlightData().Airspeed >= AircraftComp->Settings.StallSpeed);
		TestTrue("Is airborne", AircraftComp->IsAirborne());
		TestEqual("State is Airborne", (int32)AircraftComp->GetFlightState(), (int32)ESBFlightState::Airborne);
		TestTrue("Pilot has Airborne tag", PilotStateComp->HasTag(Tags.State_Movement_Flying_Airborne));
	});

	It("Should decelerate below stall speed while airborne, trigger stall state, and grant Stalling tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AircraftComp->EnterAircraft(PilotActor);
		AircraftComp->SetThrottleInput(1.0f);
		AircraftComp->UpdateFlightPhysics(1.0f, FVector::ZeroVector);

		// Cut throttle while high in the air
		AircraftComp->SetThrottleInput(0.0f);
		AircraftComp->UpdateFlightPhysics(1.0f, FVector(0.0f, 0.0f, 1000.0f));

		TestTrue("Speed dropped below stall speed", AircraftComp->GetFlightData().Airspeed < AircraftComp->Settings.StallSpeed);
		TestTrue("Is stalling", AircraftComp->IsStalling());
		TestEqual("State is Stalling", (int32)AircraftComp->GetFlightState(), (int32)ESBFlightState::Stalling);
		TestTrue("Pilot has Stalling tag", PilotStateComp->HasTag(Tags.State_Movement_Flying_Stalling));
		TestFalse("Pilot does not have Airborne tag", PilotStateComp->HasTag(Tags.State_Movement_Flying_Airborne));
	});

	It("Should enable VTOL mode to recover from stall, exit aircraft, and clear flying tags cleanly", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AircraftComp->EnterAircraft(PilotActor);
		AircraftComp->SetThrottleInput(0.0f);
		AircraftComp->UpdateFlightPhysics(1.0f, FVector(0.0f, 0.0f, 1000.0f));
		TestTrue("Stalling before VTOL", AircraftComp->IsStalling());

		AircraftComp->SetVTOLMode(true);
		TestTrue("Pilot has VTOL tag", PilotStateComp->HasTag(Tags.State_Movement_Flying_VTOL));

		AircraftComp->UpdateFlightPhysics(0.1f, FVector(0.0f, 0.0f, 1000.0f));
		TestFalse("No longer stalling in VTOL", AircraftComp->IsStalling());
		TestEqual("State is Airborne in VTOL", (int32)AircraftComp->GetFlightState(), (int32)ESBFlightState::Airborne);

		bool bExited = AircraftComp->ExitAircraft(PilotActor);

		TestTrue("Exit succeeded", bExited);
		TestFalse("Pilot no longer pilot", AircraftComp->IsPilot(PilotActor));
		TestEqual("State is Parked", (int32)AircraftComp->GetFlightState(), (int32)ESBFlightState::Parked);
		TestFalse("Pilot Flying tag removed", PilotStateComp->HasTag(Tags.State_Movement_Flying));
		TestFalse("Pilot Airborne tag removed", PilotStateComp->HasTag(Tags.State_Movement_Flying_Airborne));
		TestFalse("Pilot Stalling tag removed", PilotStateComp->HasTag(Tags.State_Movement_Flying_Stalling));
		TestFalse("Pilot VTOL tag removed", PilotStateComp->HasTag(Tags.State_Movement_Flying_VTOL));
	});
}
