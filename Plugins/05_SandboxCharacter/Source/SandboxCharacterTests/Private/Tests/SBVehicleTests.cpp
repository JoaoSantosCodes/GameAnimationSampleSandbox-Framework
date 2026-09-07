#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBVehicleComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBVehicleTestsSpec, "Sandbox.Character.VehiclesAndDynamics", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* VehicleActor;
	AActor* DriverActor;
	USBVehicleComponent* VehicleComp;
	USBStateComponent* VehicleStateComp;
	USBStateComponent* DriverStateComp;
END_DEFINE_SPEC(FSBVehicleTestsSpec)

void FSBVehicleTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		VehicleActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* VehicleRoot = NewObject<USceneComponent>(VehicleActor, TEXT("VehicleRoot"));
		VehicleActor->SetRootComponent(VehicleRoot);
		VehicleRoot->RegisterComponent();

		VehicleStateComp = NewObject<USBStateComponent>(VehicleActor, TEXT("VehicleStateComp"));
		VehicleActor->AddOwnedComponent(VehicleStateComp);

		VehicleComp = NewObject<USBVehicleComponent>(VehicleActor, TEXT("VehicleComp"));
		VehicleActor->AddOwnedComponent(VehicleComp);

		DriverActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* DriverRoot = NewObject<USceneComponent>(DriverActor, TEXT("DriverRoot"));
		DriverActor->SetRootComponent(DriverRoot);
		DriverRoot->RegisterComponent();

		DriverStateComp = NewObject<USBStateComponent>(DriverActor, TEXT("DriverStateComp"));
		DriverActor->AddOwnedComponent(DriverStateComp);

		ISBComponentInterface::Execute_OnInitialize(VehicleStateComp);
		ISBComponentInterface::Execute_OnInitialize(VehicleComp);
		ISBComponentInterface::Execute_OnInitialize(DriverStateComp);
	});

	AfterEach([this]()
	{
		if (DriverActor)
		{
			DriverActor->Destroy();
			DriverActor = nullptr;
		}

		if (VehicleActor)
		{
			VehicleActor->Destroy();
			VehicleActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should enter vehicle as driver, grant driving tags, and mark vehicle occupied", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bOccupantFired = false;
		VehicleComp->OnVehicleOccupantChanged.AddLambda([&bOccupantFired](AActor* Occ, ESBVehicleSeat Seat)
		{
			bOccupantFired = true;
		});

		bool bEntered = VehicleComp->EnterVehicle(DriverActor, ESBVehicleSeat::Driver);

		TestTrue("Enter succeeded", bEntered);
		TestTrue("Vehicle is occupied", VehicleComp->IsOccupied());
		TestTrue("Driver actor is driver", VehicleComp->IsDriver(DriverActor));
		TestTrue("Vehicle has Occupied tag", VehicleStateComp->HasTag(Tags.State_Vehicle_Occupied));
		TestTrue("Driver has Driving tag", DriverStateComp->HasTag(Tags.State_Movement_Driving));
		TestTrue("Occupant delegate fired", bOccupantFired);
	});

	It("Should start engine, accelerate forward with throttle, consume fuel, and apply Accelerating tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		VehicleComp->EnterVehicle(DriverActor, ESBVehicleSeat::Driver);
		bool bEngineStarted = VehicleComp->StartEngine();

		TestTrue("Engine started", bEngineStarted);
		TestTrue("Vehicle has EngineRunning tag", VehicleStateComp->HasTag(Tags.State_Vehicle_EngineRunning));

		VehicleComp->SetThrottleInput(1.0f);
		VehicleComp->UpdateDrivetrainPhysics(1.0f);

		TestTrue("Current speed increased", VehicleComp->GetDrivetrainData().CurrentSpeed > 0.0f);
		TestTrue("Fuel consumed", VehicleComp->GetDrivetrainData().CurrentFuel < 100.0f);
		TestTrue("Driver has Accelerating tag", DriverStateComp->HasTag(Tags.State_Movement_Driving_Accelerating));
	});

	It("Should apply handbrake, decelerate vehicle, and apply Braking tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		VehicleComp->EnterVehicle(DriverActor, ESBVehicleSeat::Driver);
		VehicleComp->StartEngine();
		VehicleComp->SetThrottleInput(1.0f);
		VehicleComp->UpdateDrivetrainPhysics(1.0f);

		const float TopSpeed = VehicleComp->GetDrivetrainData().CurrentSpeed;
		VehicleComp->SetThrottleInput(0.0f);
		VehicleComp->SetHandbrake(true);
		VehicleComp->UpdateDrivetrainPhysics(0.5f);

		TestTrue("Driver has Braking tag", DriverStateComp->HasTag(Tags.State_Movement_Driving_Braking));
		TestTrue("Vehicle decelerated under handbrake", VehicleComp->GetDrivetrainData().CurrentSpeed < TopSpeed);
	});

	It("Should exit vehicle, stop engine, clear tags, and mark vehicle unoccupied", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		VehicleComp->EnterVehicle(DriverActor, ESBVehicleSeat::Driver);
		VehicleComp->StartEngine();

		bool bExited = VehicleComp->ExitVehicle(DriverActor);

		TestTrue("Exit succeeded", bExited);
		TestFalse("Vehicle not occupied", VehicleComp->IsOccupied());
		TestFalse("Driver not driver", VehicleComp->IsDriver(DriverActor));
		TestFalse("Engine stopped", VehicleComp->GetDrivetrainData().bEngineRunning);
		TestFalse("Vehicle Occupied tag removed", VehicleStateComp->HasTag(Tags.State_Vehicle_Occupied));
		TestFalse("Vehicle EngineRunning tag removed", VehicleStateComp->HasTag(Tags.State_Vehicle_EngineRunning));
		TestFalse("Driver Driving tag removed", DriverStateComp->HasTag(Tags.State_Movement_Driving));
	});
}
