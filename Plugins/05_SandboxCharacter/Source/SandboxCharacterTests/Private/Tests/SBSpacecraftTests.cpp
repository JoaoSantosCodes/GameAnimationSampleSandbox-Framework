#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBSpacecraftComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBSpacecraftTestsSpec, "Sandbox.Character.SpacecraftAndOrbital", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* SpacecraftActor;
	AActor* PilotActor;
	USBSpacecraftComponent* SpacecraftComp;
	USBStateComponent* SpacecraftStateComp;
	USBStateComponent* PilotStateComp;
END_DEFINE_SPEC(FSBSpacecraftTestsSpec)

void FSBSpacecraftTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SpacecraftActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* SpacecraftRoot = NewObject<USceneComponent>(SpacecraftActor, TEXT("SpacecraftRoot"));
		SpacecraftActor->SetRootComponent(SpacecraftRoot);
		SpacecraftRoot->RegisterComponent();

		SpacecraftStateComp = NewObject<USBStateComponent>(SpacecraftActor, TEXT("SpacecraftStateComp"));
		SpacecraftActor->AddOwnedComponent(SpacecraftStateComp);

		SpacecraftComp = NewObject<USBSpacecraftComponent>(SpacecraftActor, TEXT("SpacecraftComp"));
		SpacecraftActor->AddOwnedComponent(SpacecraftComp);

		PilotActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* PilotRoot = NewObject<USceneComponent>(PilotActor, TEXT("PilotRoot"));
		PilotActor->SetRootComponent(PilotRoot);
		PilotRoot->RegisterComponent();

		PilotStateComp = NewObject<USBStateComponent>(PilotActor, TEXT("PilotStateComp"));
		PilotActor->AddOwnedComponent(PilotStateComp);

		ISBComponentInterface::Execute_OnInitialize(SpacecraftStateComp);
		ISBComponentInterface::Execute_OnInitialize(SpacecraftComp);
		ISBComponentInterface::Execute_OnInitialize(PilotStateComp);
	});

	AfterEach([this]()
	{
		if (PilotActor)
		{
			PilotActor->Destroy();
			PilotActor = nullptr;
		}

		if (SpacecraftActor)
		{
			SpacecraftActor->Destroy();
			SpacecraftActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should enter spacecraft as pilot, grant spaceflight tags, and set state to Drifting", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bPilotFired = false;
		SpacecraftComp->OnSpacecraftPilotChanged.AddLambda([&bPilotFired](AActor* Pilot)
		{
			bPilotFired = true;
		});

		bool bEntered = SpacecraftComp->EnterSpacecraft(PilotActor);

		TestTrue("Enter succeeded", bEntered);
		TestTrue("Is pilot", SpacecraftComp->IsPilot(PilotActor));
		TestEqual("State is Drifting", (int32)SpacecraftComp->GetFlightState(), (int32)ESBSpaceflightState::Drifting);
		TestTrue("Spacecraft has Spacecraft tag", SpacecraftStateComp->HasTag(Tags.State_Vehicle_Spacecraft));
		TestTrue("Pilot has Spaceflight tag", PilotStateComp->HasTag(Tags.State_Movement_Spaceflight));
		TestTrue("Pilot delegate fired", bPilotFired);
	});

	It("Should apply 6-DOF translation thrust, accelerate linearly, set Cruising state, and grant Cruising tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SpacecraftComp->EnterSpacecraft(PilotActor);
		SpacecraftComp->SetTranslationInput(FVector(1.0f, 0.5f, 0.0f));
		SpacecraftComp->UpdateSpaceflightPhysics(1.0f);

		TestTrue("Speed increased", SpacecraftComp->GetFlightData().CurrentSpeed > 0.0f);
		TestEqual("State is Cruising", (int32)SpacecraftComp->GetFlightState(), (int32)ESBSpaceflightState::Cruising);
		TestTrue("Pilot has Cruising tag", PilotStateComp->HasTag(Tags.State_Movement_Spaceflight_Cruising));
	});

	It("Should toggle Flight Assist Off, retain linear inertia drift, and grant FlightAssistOff tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SpacecraftComp->EnterSpacecraft(PilotActor);
		SpacecraftComp->SetTranslationInput(FVector(1.0f, 0.0f, 0.0f));
		SpacecraftComp->UpdateSpaceflightPhysics(1.0f);

		const float PreviousSpeed = SpacecraftComp->GetFlightData().CurrentSpeed;
		TestTrue("Has speed", PreviousSpeed > 0.0f);

		SpacecraftComp->SetFlightAssist(false);
		TestFalse("Flight Assist is Off", SpacecraftComp->IsFlightAssistActive());
		TestTrue("Pilot has FlightAssistOff tag", PilotStateComp->HasTag(Tags.State_Movement_Spaceflight_FlightAssistOff));

		SpacecraftComp->SetTranslationInput(FVector::ZeroVector);
		SpacecraftComp->UpdateSpaceflightPhysics(1.0f);

		TestEqual("Speed conserved without drag when Flight Assist is Off", SpacecraftComp->GetFlightData().CurrentSpeed, PreviousSpeed);
	});

	It("Should enter atmospheric reentry, trigger thermal erosion, exit spacecraft, and clear spaceflight tags cleanly", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SpacecraftComp->EnterSpacecraft(PilotActor);
		SpacecraftComp->SetAtmosphericReentry(true);
		SpacecraftComp->UpdateSpaceflightPhysics(1.0f);

		TestTrue("Heat shield eroded", SpacecraftComp->GetFlightData().HeatShieldIntegrity < 100.0f);
		TestEqual("State is Reentry", (int32)SpacecraftComp->GetFlightState(), (int32)ESBSpaceflightState::Reentry);
		TestTrue("Pilot has Reentry tag", PilotStateComp->HasTag(Tags.State_Movement_Spaceflight_Reentry));

		bool bExited = SpacecraftComp->ExitSpacecraft(PilotActor);

		TestTrue("Exit succeeded", bExited);
		TestFalse("Pilot no longer pilot", SpacecraftComp->IsPilot(PilotActor));
		TestEqual("State is Docked", (int32)SpacecraftComp->GetFlightState(), (int32)ESBSpaceflightState::Docked);
		TestFalse("Pilot Spaceflight tag removed", PilotStateComp->HasTag(Tags.State_Movement_Spaceflight));
		TestFalse("Pilot Cruising tag removed", PilotStateComp->HasTag(Tags.State_Movement_Spaceflight_Cruising));
		TestFalse("Pilot FlightAssistOff tag removed", PilotStateComp->HasTag(Tags.State_Movement_Spaceflight_FlightAssistOff));
		TestFalse("Pilot Reentry tag removed", PilotStateComp->HasTag(Tags.State_Movement_Spaceflight_Reentry));
	});
}
