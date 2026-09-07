#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBWatercraftComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBWatercraftTestsSpec, "Sandbox.Character.WatercraftAndSailing", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* VesselActor;
	AActor* PilotActor;
	USBWatercraftComponent* WatercraftComp;
	USBStateComponent* VesselStateComp;
	USBStateComponent* PilotStateComp;
END_DEFINE_SPEC(FSBWatercraftTestsSpec)

void FSBWatercraftTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		VesselActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* VesselRoot = NewObject<USceneComponent>(VesselActor, TEXT("VesselRoot"));
		VesselActor->SetRootComponent(VesselRoot);
		VesselRoot->RegisterComponent();

		VesselStateComp = NewObject<USBStateComponent>(VesselActor, TEXT("VesselStateComp"));
		VesselActor->AddOwnedComponent(VesselStateComp);

		WatercraftComp = NewObject<USBWatercraftComponent>(VesselActor, TEXT("WatercraftComp"));
		VesselActor->AddOwnedComponent(WatercraftComp);

		PilotActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* PilotRoot = NewObject<USceneComponent>(PilotActor, TEXT("PilotRoot"));
		PilotActor->SetRootComponent(PilotRoot);
		PilotRoot->RegisterComponent();

		PilotStateComp = NewObject<USBStateComponent>(PilotActor, TEXT("PilotStateComp"));
		PilotActor->AddOwnedComponent(PilotStateComp);

		ISBComponentInterface::Execute_OnInitialize(VesselStateComp);
		ISBComponentInterface::Execute_OnInitialize(WatercraftComp);
		ISBComponentInterface::Execute_OnInitialize(PilotStateComp);
	});

	AfterEach([this]()
	{
		if (PilotActor)
		{
			PilotActor->Destroy();
			PilotActor = nullptr;
		}

		if (VesselActor)
		{
			VesselActor->Destroy();
			VesselActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should enter watercraft as pilot, grant sailing tags, and set state to Drifting", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bPilotFired = false;
		WatercraftComp->OnWatercraftPilotChanged.AddLambda([&bPilotFired](AActor* Pilot)
		{
			bPilotFired = true;
		});

		bool bEntered = WatercraftComp->EnterWatercraft(PilotActor);

		TestTrue("Enter succeeded", bEntered);
		TestTrue("Is pilot", WatercraftComp->IsPilot(PilotActor));
		TestEqual("State is Drifting", (int32)WatercraftComp->GetWatercraftState(), (int32)ESBWatercraftState::Drifting);
		TestTrue("Vessel has Watercraft tag", VesselStateComp->HasTag(Tags.State_Vehicle_Watercraft));
		TestTrue("Pilot has Sailing tag", PilotStateComp->HasTag(Tags.State_Movement_Sailing));
		TestTrue("Pilot delegate fired", bPilotFired);
	});

	It("Should accelerate forward with throttle, set Cruising state, and grant Cruising tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		WatercraftComp->EnterWatercraft(PilotActor);
		WatercraftComp->SetThrottleInput(1.0f);
		WatercraftComp->UpdateWatercraftPhysics(1.0f, FVector::ZeroVector);

		TestTrue("Speed increased", WatercraftComp->GetNavigationData().CurrentSpeed > 0.0f);
		TestEqual("State is Cruising", (int32)WatercraftComp->GetWatercraftState(), (int32)ESBWatercraftState::Cruising);
		TestTrue("Pilot has Cruising tag", PilotStateComp->HasTag(Tags.State_Movement_Sailing_Cruising));
	});

	It("Should drop anchor, lock current speed to zero, set state to Anchored, and grant Anchored tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		WatercraftComp->EnterWatercraft(PilotActor);
		WatercraftComp->SetThrottleInput(1.0f);
		WatercraftComp->UpdateWatercraftPhysics(1.0f, FVector::ZeroVector);

		bool bDropped = WatercraftComp->DropAnchor();

		TestTrue("Drop anchor succeeded", bDropped);
		TestTrue("Is anchored", WatercraftComp->IsAnchored());
		TestEqual("State is Anchored", (int32)WatercraftComp->GetWatercraftState(), (int32)ESBWatercraftState::Anchored);

		WatercraftComp->UpdateWatercraftPhysics(1.0f, FVector::ZeroVector);

		TestEqual("Speed locked to 0 when anchored", WatercraftComp->GetNavigationData().CurrentSpeed, 0.0f);
		TestTrue("Pilot has Anchored tag", PilotStateComp->HasTag(Tags.State_Movement_Sailing_Anchored));
		TestFalse("Pilot does not have Cruising tag", PilotStateComp->HasTag(Tags.State_Movement_Sailing_Cruising));
	});

	It("Should raise anchor, exit watercraft, and clear sailing tags cleanly", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		WatercraftComp->EnterWatercraft(PilotActor);
		WatercraftComp->DropAnchor();

		bool bRaised = WatercraftComp->RaiseAnchor();
		TestTrue("Raise anchor succeeded", bRaised);
		TestFalse("Not anchored", WatercraftComp->IsAnchored());

		bool bExited = WatercraftComp->ExitWatercraft(PilotActor);

		TestTrue("Exit succeeded", bExited);
		TestFalse("Pilot no longer pilot", WatercraftComp->IsPilot(PilotActor));
		TestEqual("State is Docked", (int32)WatercraftComp->GetWatercraftState(), (int32)ESBWatercraftState::Docked);
		TestFalse("Pilot Sailing tag removed", PilotStateComp->HasTag(Tags.State_Movement_Sailing));
		TestFalse("Pilot Cruising tag removed", PilotStateComp->HasTag(Tags.State_Movement_Sailing_Cruising));
		TestFalse("Pilot Anchored tag removed", PilotStateComp->HasTag(Tags.State_Movement_Sailing_Anchored));
	});
}
