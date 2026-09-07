#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBSpaceElevatorComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBSpaceElevatorTestsSpec, "Sandbox.Inventory.SpaceElevator", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* ElevatorActor;
	USBSpaceElevatorComponent* ElevatorComp;
	USBStateComponent* ElevatorStateComp;
END_DEFINE_SPEC(FSBSpaceElevatorTestsSpec)

void FSBSpaceElevatorTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ElevatorActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(ElevatorActor, TEXT("Root"));
		ElevatorActor->SetRootComponent(Root);
		Root->RegisterComponent();

		ElevatorStateComp = NewObject<USBStateComponent>(ElevatorActor, TEXT("ElevatorStateComp"));
		ElevatorActor->AddOwnedComponent(ElevatorStateComp);

		ElevatorComp = NewObject<USBSpaceElevatorComponent>(ElevatorActor, TEXT("ElevatorComp"));
		ElevatorActor->AddOwnedComponent(ElevatorComp);

		ISBComponentInterface::Execute_OnInitialize(ElevatorStateComp);
		ISBComponentInterface::Execute_OnInitialize(ElevatorComp);
	});

	AfterEach([this]()
	{
		if (ElevatorActor)
		{
			ElevatorActor->Destroy();
			ElevatorActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should accept phase requirement items, validate quota completion, and block launch if incomplete", [this]()
	{
		ElevatorComp->SetupSpaceElevator(3, 50.0f);

		TMap<FName, int32> Phase1Reqs;
		Phase1Reqs.Add(FName(TEXT("ReinforcedSteel")), 100);
		Phase1Reqs.Add(FName(TEXT("QuantumProcessor")), 20);

		ElevatorComp->ConfigurePhaseRequirement(1, FName(TEXT("Phase_1_Anchor")), Phase1Reqs);

		// Deposit partial
		int32 AddedSteel = ElevatorComp->DepositPhaseItem(FName(TEXT("ReinforcedSteel")), 50);
		TestEqual("Steel added", AddedSteel, 50);
		TestFalse("Quota not met yet", ElevatorComp->IsCurrentPhaseRequirementMet());
		TestFalse("Cannot launch with incomplete quota", ElevatorComp->LaunchOrbitalDelivery());

		// Complete remaining quota
		ElevatorComp->DepositPhaseItem(FName(TEXT("ReinforcedSteel")), 50);
		ElevatorComp->DepositPhaseItem(FName(TEXT("QuantumProcessor")), 20);

		TestTrue("Quota fully met", ElevatorComp->IsCurrentPhaseRequirementMet());
		TestEqual("Total steel deposited", ElevatorComp->GetDepositedItemCount(1, FName(TEXT("ReinforcedSteel"))), 100);
		TestEqual("Total processors deposited", ElevatorComp->GetDepositedItemCount(1, FName(TEXT("QuantumProcessor"))), 20);
	});

	It("Should launch orbital delivery, ascend pod along tether cable, and apply Ascending tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ElevatorComp->SetupSpaceElevator(3, 50.0f);

		TMap<FName, int32> Reqs;
		Reqs.Add(FName(TEXT("IronPlate")), 10);
		ElevatorComp->ConfigurePhaseRequirement(1, FName(TEXT("Phase_1")), Reqs);
		ElevatorComp->DepositPhaseItem(FName(TEXT("IronPlate")), 10);

		bool bLaunched = ElevatorComp->LaunchOrbitalDelivery();
		TestTrue("Launch succeeded", bLaunched);
		TestEqual("State is Ascending", (int32)ElevatorComp->GetElevatorState(), (int32)ESBSpaceElevatorState::Ascending);
		TestTrue("Has Ascending tag", ElevatorStateComp->HasTag(Tags.State_SpaceElevator_Ascending));

		// Ascend tick
		ElevatorComp->SimulateElevatorTick(2.0f);
		TestTrue("Pod altitude ascended", ElevatorComp->GetSpaceElevatorData().PodAltitudeAlpha > 0.0f);
	});

	It("Should dock at orbital station, complete phase, broadcast completion delegate, and descend to base", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ElevatorComp->SetupSpaceElevator(3, 50.0f);

		TMap<FName, int32> Reqs;
		Reqs.Add(FName(TEXT("IronPlate")), 10);
		ElevatorComp->ConfigurePhaseRequirement(1, FName(TEXT("Phase_1_Anchor")), Reqs);
		ElevatorComp->DepositPhaseItem(FName(TEXT("IronPlate")), 10);

		ElevatorComp->LaunchOrbitalDelivery();

		// Ascend to low orbit (1.0)
		ElevatorComp->SimulateElevatorTick(5.5f);
		TestEqual("Docked at orbit", (int32)ElevatorComp->GetElevatorState(), (int32)ESBSpaceElevatorState::DockedAtOrbitalStation);
		TestTrue("Has PhaseCompleted tag", ElevatorStateComp->HasTag(Tags.State_SpaceElevator_PhaseCompleted));

		// Wait in orbit for delivery transfer (2.0s) -> Descending
		ElevatorComp->SimulateElevatorTick(2.5f);
		TestEqual("Payload delivered -> Descending", (int32)ElevatorComp->GetElevatorState(), (int32)ESBSpaceElevatorState::Descending);

		// Descend back to ground base (0.0) -> Idle at Phase 2
		ElevatorComp->SimulateElevatorTick(4.0f);
		TestEqual("Landed -> Idle", (int32)ElevatorComp->GetElevatorState(), (int32)ESBSpaceElevatorState::Idle);
		TestEqual("Current phase advanced to Phase 2", ElevatorComp->GetSpaceElevatorData().CurrentPhaseIndex, 2);
	});

	It("Should stall pod ascent along cable if power supply to space elevator is cut", [this]()
	{
		ElevatorComp->SetupSpaceElevator(3, 50.0f);

		TMap<FName, int32> Reqs;
		Reqs.Add(FName(TEXT("IronPlate")), 10);
		ElevatorComp->ConfigurePhaseRequirement(1, FName(TEXT("Phase_1")), Reqs);
		ElevatorComp->DepositPhaseItem(FName(TEXT("IronPlate")), 10);

		ElevatorComp->LaunchOrbitalDelivery();
		ElevatorComp->SimulateElevatorTick(1.0f);

		float AltBefore = ElevatorComp->GetSpaceElevatorData().PodAltitudeAlpha;
		TestTrue("Ascending with power", AltBefore > 0.0f);

		// Cut electrical power supply
		ElevatorComp->SetPowerSupplied(false);
		ElevatorComp->SimulateElevatorTick(2.0f);

		TestEqual("Pod stalled in place during blackout", ElevatorComp->GetSpaceElevatorData().PodAltitudeAlpha, AltBefore);
	});
}
