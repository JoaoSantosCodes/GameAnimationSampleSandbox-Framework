// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBRailNetworkComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBRailNetworkTestsSpec, "Sandbox.Inventory.RailNetwork", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TrainActor;
	USBRailNetworkComponent* RailComp;
	USBStateComponent* TrainStateComp;
END_DEFINE_SPEC(FSBRailNetworkTestsSpec)

void FSBRailNetworkTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		TrainActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(TrainActor, TEXT("Root"));
		TrainActor->SetRootComponent(Root);
		Root->RegisterComponent();

		TrainStateComp = NewObject<USBStateComponent>(TrainActor, TEXT("TrainStateComp"));
		TrainActor->AddOwnedComponent(TrainStateComp);

		RailComp = NewObject<USBRailNetworkComponent>(TrainActor, TEXT("RailComp"));
		TrainActor->AddOwnedComponent(RailComp);

		ISBComponentInterface::Execute_OnInitialize(TrainStateComp);
		ISBComponentInterface::Execute_OnInitialize(RailComp);
	});

	AfterEach([this]()
	{
		if (TrainActor)
		{
			TrainActor->Destroy();
			TrainActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should travel along railroad track, accelerate up to max speed, and grant Traveling tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		RailComp->SetupTrainConsist(FName(TEXT("ExpressTrain")), 120.0f);

		FSBRailWagonData Locomotive;
		Locomotive.WagonId = FName(TEXT("Loco_01"));
		Locomotive.WagonType = ESBWagonType::LocomotiveEngine;
		RailComp->AddWagon(Locomotive);

		FSBRailWagonData CargoWagon;
		CargoWagon.WagonId = FName(TEXT("Cargo_01"));
		CargoWagon.WagonType = ESBWagonType::FreightCargo;
		CargoWagon.CargoCapacity = 100;
		RailComp->AddWagon(CargoWagon);

		RailComp->StartTravel();

		TestEqual("Movement state is Traveling", (int32)RailComp->GetMovementState(), (int32)ESBTrainMovementState::Traveling);
		TestTrue("Has Traveling tag", TrainStateComp->HasTag(Tags.State_Rail_Traveling));

		RailComp->SimulateRailTick(1.0f);

		TestEqual("Current speed accelerated to 15 km/h", RailComp->GetTrainConsist().CurrentSpeed, 15.0f);
		TestTrue("Track progress advanced", RailComp->GetTrainConsist().CurrentTrackProgressAlpha > 0.0f);
	});

	It("Should decelerate and wait at red signal when upcoming block is occupied, granting WaitingSignal tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		RailComp->SetupTrainConsist(FName(TEXT("ExpressTrain")), 120.0f);
		RailComp->StartTravel();
		RailComp->SimulateRailTick(1.0f); // Accelerates to 15 km/h

		// Next block is occupied by another train
		RailComp->RegisterRailBlock(1);
		RailComp->SetBlockOccupied(1, true, FName(TEXT("BlockerTrain")));

		// Simulate deceleration towards red signal
		RailComp->SimulateRailTick(1.0f); // 15 - 25 <= 0 -> speed hits 0 -> WaitingSignal

		TestEqual("Movement state is WaitingSignal", (int32)RailComp->GetMovementState(), (int32)ESBTrainMovementState::WaitingSignal);
		TestTrue("Has WaitingSignal tag", TrainStateComp->HasTag(Tags.State_Rail_WaitingSignal));

		// Signal turns green
		RailComp->SetBlockOccupied(1, false, NAME_None);
		RailComp->SimulateRailTick(0.1f);

		TestEqual("Movement state resumed to Traveling", (int32)RailComp->GetMovementState(), (int32)ESBTrainMovementState::Traveling);
		TestTrue("Has Traveling tag", TrainStateComp->HasTag(Tags.State_Rail_Traveling));
	});

	It("Should stop at scheduled station, transition to Loading, load cargo into wagon, and depart on timeout", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		RailComp->SetupTrainConsist(FName(TEXT("ExpressTrain")), 120.0f);

		FSBRailWagonData CargoWagon;
		CargoWagon.WagonId = FName(TEXT("Cargo_01"));
		CargoWagon.WagonType = ESBWagonType::FreightCargo;
		CargoWagon.CargoCapacity = 100;
		RailComp->AddWagon(CargoWagon);

		RailComp->AddStationToSchedule(FName(TEXT("IronMineStation")));
		RailComp->StartTravel();

		// Advance train to 100% of current segment to trigger arrival at IronMineStation
		RailComp->SimulateRailTick(24.0f); // Covers distance to advance block

		TestEqual("Movement state is Loading", (int32)RailComp->GetMovementState(), (int32)ESBTrainMovementState::Loading);
		TestTrue("Has Loading tag", TrainStateComp->HasTag(Tags.State_Rail_Loading));

		// Load cargo into wagon
		int32 Loaded = RailComp->LoadCargoIntoWagon(0, FName(TEXT("IronOre")), 60);
		TestEqual("Loaded 60 IronOre", Loaded, 60);
		TestEqual("Wagon has 60 IronOre", RailComp->GetWagonCargoCount(0, FName(TEXT("IronOre"))), 60);

		// Advance past station wait duration (3.0s)
		RailComp->SimulateRailTick(3.5f);

		TestEqual("Train departed and resumed Traveling", (int32)RailComp->GetMovementState(), (int32)ESBTrainMovementState::Traveling);
		TestTrue("Has Traveling tag", TrainStateComp->HasTag(Tags.State_Rail_Traveling));
	});

	It("Should stop at destination station with cargo, enter Unloading state, unload cargo, and depart", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		RailComp->SetupTrainConsist(FName(TEXT("ExpressTrain")), 120.0f);

		FSBRailWagonData CargoWagon;
		CargoWagon.WagonId = FName(TEXT("Cargo_01"));
		CargoWagon.WagonType = ESBWagonType::FreightCargo;
		CargoWagon.CargoCapacity = 100;
		RailComp->AddWagon(CargoWagon);

		RailComp->LoadCargoIntoWagon(0, FName(TEXT("IronOre")), 60);
		RailComp->AddStationToSchedule(FName(TEXT("SmelterHubStation")));
		RailComp->StartTravel();

		// Advance to arrive at station
		RailComp->SimulateRailTick(24.0f);

		TestEqual("Movement state is Unloading", (int32)RailComp->GetMovementState(), (int32)ESBTrainMovementState::Unloading);
		TestTrue("Has Unloading tag", TrainStateComp->HasTag(Tags.State_Rail_Unloading));

		// Unload cargo
		int32 Unloaded = RailComp->UnloadCargoFromWagon(0, FName(TEXT("IronOre")), 60);
		TestEqual("Unloaded 60 IronOre", Unloaded, 60);
		TestEqual("Wagon is now empty", RailComp->GetWagonCargoCount(0, FName(TEXT("IronOre"))), 0);

		// Advance past station wait duration
		RailComp->SimulateRailTick(3.5f);

		TestEqual("Train departed and resumed Traveling", (int32)RailComp->GetMovementState(), (int32)ESBTrainMovementState::Traveling);
		TestTrue("Has Traveling tag", TrainStateComp->HasTag(Tags.State_Rail_Traveling));
	});
}
