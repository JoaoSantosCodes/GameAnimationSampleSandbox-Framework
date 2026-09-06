#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBCargoDroneNetworkComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBCargoDroneNetworkTestsSpec, "Sandbox.Inventory.CargoDroneNetwork", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* DroneActor;
	USBCargoDroneNetworkComponent* DroneComp;
	USBStateComponent* DroneStateComp;
END_DEFINE_SPEC(FSBCargoDroneNetworkTestsSpec)

void FSBCargoDroneNetworkTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		DroneActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(DroneActor, TEXT("Root"));
		DroneActor->SetRootComponent(Root);
		Root->RegisterComponent();

		DroneStateComp = NewObject<USBStateComponent>(DroneActor, TEXT("DroneStateComp"));
		DroneActor->AddOwnedComponent(DroneStateComp);

		DroneComp = NewObject<USBCargoDroneNetworkComponent>(DroneActor, TEXT("DroneComp"));
		DroneActor->AddOwnedComponent(DroneComp);

		ISBComponentInterface::Execute_OnInitialize(DroneStateComp);
		ISBComponentInterface::Execute_OnInitialize(DroneComp);
	});

	AfterEach([this]()
	{
		if (DroneActor)
		{
			DroneActor->Destroy();
			DroneActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should takeoff from origin port, transition to InFlight, and consume battery power with InFlight tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		DroneComp->SetupDrone(FName(TEXT("Drone_Alpha")), ESBCargoDroneModel::LightCourier, 120.0f, 100.0f, 50);
		DroneComp->RegisterDronePort(FName(TEXT("Port_HQ")), FVector::ZeroVector, true);
		DroneComp->RegisterDronePort(FName(TEXT("Port_Outpost")), FVector(5000.0f, 0.0f, 0.0f), true);
		DroneComp->SetFlightRoute(FName(TEXT("Port_HQ")), FName(TEXT("Port_Outpost")));

		// Load cargo
		int32 Loaded = DroneComp->LoadCargoIntoDrone(FName(TEXT("IronOre")), 30);
		TestEqual("Cargo loaded", Loaded, 30);
		TestEqual("Cargo bay count", DroneComp->GetDroneCargoCount(FName(TEXT("IronOre"))), 30);

		// Dispatch
		bool bDispatched = DroneComp->DispatchDrone();
		TestTrue("Drone dispatched", bDispatched);
		TestEqual("State is TakingOff", (int32)DroneComp->GetFlightState(), (int32)ESBCargoDroneFlightState::TakingOff);
		TestTrue("Has TakingOff tag", DroneStateComp->HasTag(Tags.State_Drone_TakingOff));

		// Advance takeoff duration (2.0s)
		DroneComp->SimulateDroneTick(2.5f);
		TestEqual("State transitioned to InFlight", (int32)DroneComp->GetFlightState(), (int32)ESBCargoDroneFlightState::InFlight);
		TestTrue("Has InFlight tag", DroneStateComp->HasTag(Tags.State_Drone_InFlight));
		TestTrue("Battery consumed in flight", DroneComp->GetDroneData().CurrentBattery < 100.0f);
	});

	It("Should navigate flight route, land at destination port, and broadcast arrival delegate", [this]()
	{
		DroneComp->SetupDrone(FName(TEXT("Drone_Beta")), ESBCargoDroneModel::HeavyLiftDrone, 600.0f, 100.0f, 100);
		DroneComp->RegisterDronePort(FName(TEXT("Port_HQ")), FVector::ZeroVector, true);
		DroneComp->RegisterDronePort(FName(TEXT("Port_Outpost")), FVector(5000.0f, 0.0f, 0.0f), true);
		DroneComp->SetFlightRoute(FName(TEXT("Port_HQ")), FName(TEXT("Port_Outpost")));

		DroneComp->DispatchDrone();
		DroneComp->SimulateDroneTick(2.0f); // Complete Takeoff -> InFlight
		TestEqual("In Flight", (int32)DroneComp->GetFlightState(), (int32)ESBCargoDroneFlightState::InFlight);

		// Advance flight corridor progress to 100% -> Landing
		DroneComp->SimulateDroneTick(1.0f);
		TestEqual("Approached port -> Landing", (int32)DroneComp->GetFlightState(), (int32)ESBCargoDroneFlightState::Landing);

		// Complete landing duration (2.0s) -> IdleAtPort
		DroneComp->SimulateDroneTick(2.0f);
		TestEqual("Touchdown -> IdleAtPort", (int32)DroneComp->GetFlightState(), (int32)ESBCargoDroneFlightState::IdleAtPort);
		TestEqual("New home port is Port_Outpost", DroneComp->GetDroneData().HomePortId, FName(TEXT("Port_Outpost")));
		TestEqual("Trips completed count is 1", DroneComp->GetDroneData().TotalTripsCompleted, 1);
	});

	It("Should dock at recharge port and replenish depleted battery up to 100%", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		DroneComp->SetupDrone(FName(TEXT("Drone_Gamma")), ESBCargoDroneModel::LightCourier, 120.0f, 100.0f, 50);
		DroneComp->RegisterDronePort(FName(TEXT("Port_HQ")), FVector::ZeroVector, true);
		DroneComp->SetFlightRoute(FName(TEXT("Port_HQ")), FName(TEXT("Port_HQ")));

		// Simulate returned with 40% battery
		DroneComp->LoadCargoIntoDrone(FName(TEXT("CopperIngot")), 10);
		
		// Dispatch and force battery down
		DroneComp->DispatchDrone();
		// A velocidade 120 rende ProgressRate = 120/60*0.1 = 0.2/s, ou seja 5s de voo. Os
		// ticks anteriores (1s) assumiam a velocidade 600 do outro teste e o drone nunca
		// chegava a pousar -- seguia InFlight ate o fim do bloco.
		DroneComp->SimulateDroneTick(2.0f); // Takeoff concluido -> InFlight
		DroneComp->SimulateDroneTick(5.0f); // Rota completa -> Landing
		DroneComp->SimulateDroneTick(2.0f); // Touchdown -> IdleAtPort

		// Docado com bateria abaixo do maximo -> inicia recarga
		DroneComp->SimulateDroneTick(0.1f);
		TestEqual("Started recharging", (int32)DroneComp->GetFlightState(), (int32)ESBCargoDroneFlightState::Recharging);
		TestTrue("Has Recharging tag", DroneStateComp->HasTag(Tags.State_Drone_Recharging));

		// Recarrega ate o maximo (a recarga satura em MaxBattery e volta para IdleAtPort)
		DroneComp->SimulateDroneTick(20.0f);
		TestEqual("Fully charged -> IdleAtPort", (int32)DroneComp->GetFlightState(), (int32)ESBCargoDroneFlightState::IdleAtPort);
		TestEqual("Battery is 100%", DroneComp->GetDroneData().CurrentBattery, 100.0f);
	});

	It("Should initiate emergency return to home base when battery falls below critical reserve threshold", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		DroneComp->SetupDrone(FName(TEXT("Drone_Delta")), ESBCargoDroneModel::LongRangeHexacopter, 60.0f, 100.0f, 80);
		DroneComp->RegisterDronePort(FName(TEXT("Port_Base")), FVector::ZeroVector, true);
		DroneComp->RegisterDronePort(FName(TEXT("Port_Remote")), FVector(20000.0f, 0.0f, 0.0f), true);
		DroneComp->SetFlightRoute(FName(TEXT("Port_Base")), FName(TEXT("Port_Remote")));

		DroneComp->DispatchDrone();
		DroneComp->SimulateDroneTick(2.0f); // Takeoff done -> InFlight
		TestEqual("Cruising", (int32)DroneComp->GetFlightState(), (int32)ESBCargoDroneFlightState::InFlight);

		// Discharge battery past critical 25% threshold (ex: simulate long headwind drain)
		DroneComp->SimulateDroneTick(36.0f); // 36s * 2%/s = 72% drain -> 28% - 4% = 24% <= 25%

		TestEqual("Emergency Return Triggered", (int32)DroneComp->GetFlightState(), (int32)ESBCargoDroneFlightState::LowBatteryReturn);
		TestTrue("Has LowBattery tag", DroneStateComp->HasTag(Tags.State_Drone_LowBattery));
		TestEqual("Target Port rerouted back to home", DroneComp->GetDroneData().TargetPortId, FName(TEXT("Port_Base")));
	});
}
