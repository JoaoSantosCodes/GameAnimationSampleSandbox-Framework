#include "Components/SBVehicleComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBVehicleComponent::USBVehicleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBVehicleComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBVehicleComponent::OnShutdown_Implementation()
{
	TArray<FSBVehicleSeatOccupant> TempOccupants = Occupants;
	for (const FSBVehicleSeatOccupant& Occ : TempOccupants)
	{
		if (Occ.OccupantActor.IsValid())
		{
			ExitVehicle(Occ.OccupantActor.Get());
		}
	}
	StopEngine();
}

bool USBVehicleComponent::EnterVehicle(AActor* InActor, ESBVehicleSeat InSeat)
{
	if (!InActor || IsSeatOccupied(InSeat))
	{
		return false;
	}

	for (const FSBVehicleSeatOccupant& Occ : Occupants)
	{
		if (Occ.OccupantActor.Get() == InActor)
		{
			return false;
		}
	}

	FSBVehicleSeatOccupant NewOcc;
	NewOcc.OccupantActor = InActor;
	NewOcc.Seat = InSeat;
	Occupants.Add(NewOcc);

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Vehicle_Occupied);
	}

	if (InSeat == ESBVehicleSeat::Driver)
	{
		if (USBStateComponent* OccState = InActor->FindComponentByClass<USBStateComponent>())
		{
			OccState->AddTag(Tags.State_Movement_Driving);
		}
	}

	OnVehicleOccupantChanged.Broadcast(InActor, InSeat);
	return true;
}

bool USBVehicleComponent::ExitVehicle(AActor* InActor)
{
	if (!InActor)
	{
		return false;
	}

	int32 FoundIdx = INDEX_NONE;
	for (int32 i = 0; i < Occupants.Num(); ++i)
	{
		if (Occupants[i].OccupantActor.Get() == InActor)
		{
			FoundIdx = i;
			break;
		}
	}

	if (FoundIdx == INDEX_NONE)
	{
		return false;
	}

	const ESBVehicleSeat RemovedSeat = Occupants[FoundIdx].Seat;
	Occupants.RemoveAt(FoundIdx);

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (RemovedSeat == ESBVehicleSeat::Driver)
	{
		if (USBStateComponent* OccState = InActor->FindComponentByClass<USBStateComponent>())
		{
			OccState->RemoveTag(Tags.State_Movement_Driving);
			OccState->RemoveTag(Tags.State_Movement_Driving_Accelerating);
			OccState->RemoveTag(Tags.State_Movement_Driving_Braking);
			OccState->RemoveTag(Tags.State_Movement_Driving_Reverse);
		}
		StopEngine();
	}

	if (Occupants.Num() == 0 && CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Vehicle_Occupied);
	}

	OnVehicleOccupantChanged.Broadcast(InActor, RemovedSeat);
	return true;
}

bool USBVehicleComponent::StartEngine()
{
	if (DrivetrainData.CurrentFuel <= 0.0f || DrivetrainData.bEngineRunning)
	{
		return false;
	}

	DrivetrainData.bEngineRunning = true;

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (CachedStateComp.IsValid())
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();
		CachedStateComp->AddTag(Tags.State_Vehicle_EngineRunning);
	}

	OnVehicleEngineStateChanged.Broadcast(true);
	return true;
}

bool USBVehicleComponent::StopEngine()
{
	if (!DrivetrainData.bEngineRunning)
	{
		return false;
	}

	DrivetrainData.bEngineRunning = false;
	DrivetrainData.ThrottleInput = 0.0f;

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (CachedStateComp.IsValid())
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();
		CachedStateComp->RemoveTag(Tags.State_Vehicle_EngineRunning);
	}

	SyncDrivetrainTags();
	OnVehicleEngineStateChanged.Broadcast(false);
	return true;
}

void USBVehicleComponent::SetThrottleInput(float InThrottle)
{
	DrivetrainData.ThrottleInput = FMath::Clamp(InThrottle, -1.0f, 1.0f);
}

void USBVehicleComponent::SetSteeringInput(float InSteering)
{
	DrivetrainData.SteeringInput = FMath::Clamp(InSteering, -1.0f, 1.0f);
}

void USBVehicleComponent::SetHandbrake(bool bActive)
{
	DrivetrainData.bHandbrakeActive = bActive;
}

void USBVehicleComponent::UpdateDrivetrainPhysics(float DeltaTime)
{
	if (!DrivetrainData.bEngineRunning)
	{
		if (DrivetrainData.CurrentSpeed > 0.0f)
		{
			DrivetrainData.CurrentSpeed = FMath::Max(0.0f, DrivetrainData.CurrentSpeed - (Settings.NaturalFriction * DeltaTime));
		}
		else if (DrivetrainData.CurrentSpeed < 0.0f)
		{
			DrivetrainData.CurrentSpeed = FMath::Min(0.0f, DrivetrainData.CurrentSpeed + (Settings.NaturalFriction * DeltaTime));
		}
		SyncDrivetrainTags();
		return;
	}

	if (DrivetrainData.bHandbrakeActive)
	{
		if (DrivetrainData.CurrentSpeed > 0.0f)
		{
			DrivetrainData.CurrentSpeed = FMath::Max(0.0f, DrivetrainData.CurrentSpeed - (Settings.BrakingDeceleration * DeltaTime));
		}
		else if (DrivetrainData.CurrentSpeed < 0.0f)
		{
			DrivetrainData.CurrentSpeed = FMath::Min(0.0f, DrivetrainData.CurrentSpeed + (Settings.BrakingDeceleration * DeltaTime));
		}
	}
	else if (DrivetrainData.ThrottleInput > 0.0f)
	{
		DrivetrainData.CurrentSpeed = FMath::Min(Settings.MaxForwardSpeed, DrivetrainData.CurrentSpeed + (Settings.AccelerationRate * DrivetrainData.ThrottleInput * DeltaTime));
	}
	else if (DrivetrainData.ThrottleInput < 0.0f)
	{
		if (DrivetrainData.CurrentSpeed > 0.0f)
		{
			DrivetrainData.CurrentSpeed = FMath::Max(0.0f, DrivetrainData.CurrentSpeed - (Settings.BrakingDeceleration * FMath::Abs(DrivetrainData.ThrottleInput) * DeltaTime));
		}
		else
		{
			DrivetrainData.CurrentSpeed = FMath::Max(-Settings.MaxReverseSpeed, DrivetrainData.CurrentSpeed + (Settings.AccelerationRate * DrivetrainData.ThrottleInput * DeltaTime));
		}
	}
	else
	{
		if (DrivetrainData.CurrentSpeed > 0.0f)
		{
			DrivetrainData.CurrentSpeed = FMath::Max(0.0f, DrivetrainData.CurrentSpeed - (Settings.NaturalFriction * DeltaTime));
		}
		else if (DrivetrainData.CurrentSpeed < 0.0f)
		{
			DrivetrainData.CurrentSpeed = FMath::Min(0.0f, DrivetrainData.CurrentSpeed + (Settings.NaturalFriction * DeltaTime));
		}
	}

	const float FuelUsed = Settings.FuelConsumptionRate * (1.0f + FMath::Abs(DrivetrainData.ThrottleInput)) * DeltaTime;
	DrivetrainData.CurrentFuel = FMath::Max(0.0f, DrivetrainData.CurrentFuel - FuelUsed);
	OnVehicleFuelChanged.Broadcast(DrivetrainData.CurrentFuel);

	if (DrivetrainData.CurrentFuel <= 0.0f)
	{
		StopEngine();
	}

	SyncDrivetrainTags();
}

bool USBVehicleComponent::IsDriver(const AActor* InActor) const
{
	for (const FSBVehicleSeatOccupant& Occ : Occupants)
	{
		if (Occ.Seat == ESBVehicleSeat::Driver && Occ.OccupantActor.Get() == InActor)
		{
			return true;
		}
	}
	return false;
}

AActor* USBVehicleComponent::GetDriver() const
{
	for (const FSBVehicleSeatOccupant& Occ : Occupants)
	{
		if (Occ.Seat == ESBVehicleSeat::Driver)
		{
			return Occ.OccupantActor.Get();
		}
	}
	return nullptr;
}

bool USBVehicleComponent::IsSeatOccupied(ESBVehicleSeat InSeat) const
{
	for (const FSBVehicleSeatOccupant& Occ : Occupants)
	{
		if (Occ.Seat == InSeat)
		{
			return true;
		}
	}
	return false;
}

void USBVehicleComponent::SyncDrivetrainTags()
{
	AActor* DriverActor = GetDriver();
	if (!DriverActor)
	{
		return;
	}

	USBStateComponent* DriverState = DriverActor->FindComponentByClass<USBStateComponent>();
	if (!DriverState)
	{
		return;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	DriverState->RemoveTag(Tags.State_Movement_Driving_Accelerating);
	DriverState->RemoveTag(Tags.State_Movement_Driving_Braking);
	DriverState->RemoveTag(Tags.State_Movement_Driving_Reverse);

	if (DrivetrainData.bHandbrakeActive || (DrivetrainData.ThrottleInput < 0.0f && DrivetrainData.CurrentSpeed > 0.0f))
	{
		DriverState->AddTag(Tags.State_Movement_Driving_Braking);
	}
	else if (DrivetrainData.ThrottleInput > 0.0f)
	{
		DriverState->AddTag(Tags.State_Movement_Driving_Accelerating);
	}
	else if (DrivetrainData.CurrentSpeed < 0.0f)
	{
		DriverState->AddTag(Tags.State_Movement_Driving_Reverse);
	}
}
