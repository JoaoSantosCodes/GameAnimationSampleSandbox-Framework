// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBAircraftComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBAircraftComponent::USBAircraftComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBAircraftComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBAircraftComponent::OnShutdown_Implementation()
{
	if (CurrentPilot.IsValid())
	{
		ExitAircraft(CurrentPilot.Get());
	}
}

bool USBAircraftComponent::EnterAircraft(AActor* InPilot)
{
	if (!InPilot || CurrentPilot.IsValid())
	{
		return false;
	}

	CurrentPilot = InPilot;
	FlightState = ESBFlightState::Taxiing;

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Vehicle_Aircraft);
	}

	if (USBStateComponent* PilotState = InPilot->FindComponentByClass<USBStateComponent>())
	{
		PilotState->AddTag(Tags.State_Movement_Flying);
	}

	OnAircraftPilotChanged.Broadcast(InPilot);
	OnFlightStateChanged.Broadcast(FlightState);
	return true;
}

bool USBAircraftComponent::ExitAircraft(AActor* InPilot)
{
	if (!InPilot || CurrentPilot.Get() != InPilot)
	{
		return false;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (USBStateComponent* PilotState = InPilot->FindComponentByClass<USBStateComponent>())
	{
		PilotState->RemoveTag(Tags.State_Movement_Flying);
		PilotState->RemoveTag(Tags.State_Movement_Flying_Airborne);
		PilotState->RemoveTag(Tags.State_Movement_Flying_Stalling);
		PilotState->RemoveTag(Tags.State_Movement_Flying_VTOL);
	}

	CurrentPilot = nullptr;
	FlightData.ThrottleInput = 0.0f;
	FlightData.Airspeed = 0.0f;
	FlightData.bIsAirborne = false;
	FlightData.bIsStalling = false;
	FlightState = ESBFlightState::Parked;

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Movement_Flying_Airborne);
		CachedStateComp->RemoveTag(Tags.State_Movement_Flying_Stalling);
		CachedStateComp->RemoveTag(Tags.State_Movement_Flying_VTOL);
	}

	OnAircraftPilotChanged.Broadcast(nullptr);
	OnFlightStateChanged.Broadcast(FlightState);
	return true;
}

void USBAircraftComponent::SetThrottleInput(float InThrottle)
{
	FlightData.ThrottleInput = FMath::Clamp(InThrottle, -1.0f, 1.0f);
}

void USBAircraftComponent::SetFlightControls(float InPitch, float InRoll, float InYaw)
{
	FlightData.PitchInput = FMath::Clamp(InPitch, -1.0f, 1.0f);
	FlightData.RollInput = FMath::Clamp(InRoll, -1.0f, 1.0f);
	FlightData.YawInput = FMath::Clamp(InYaw, -1.0f, 1.0f);
}

void USBAircraftComponent::SetVTOLMode(bool bEnable)
{
	FlightData.bVTOLMode = bEnable;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (bEnable)
	{
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Flying_VTOL);
		if (AActor* P = CurrentPilot.Get()) if (USBStateComponent* PS = P->FindComponentByClass<USBStateComponent>()) PS->AddTag(Tags.State_Movement_Flying_VTOL);
	}
	else
	{
		if (CachedStateComp.IsValid()) CachedStateComp->RemoveTag(Tags.State_Movement_Flying_VTOL);
		if (AActor* P = CurrentPilot.Get()) if (USBStateComponent* PS = P->FindComponentByClass<USBStateComponent>()) PS->RemoveTag(Tags.State_Movement_Flying_VTOL);
	}
}

void USBAircraftComponent::UpdateFlightPhysics(float DeltaTime, const FVector& CurrentActorLocation)
{
	FlightData.Altitude = CurrentActorLocation.Z;

	if (FlightData.ThrottleInput > 0.0f)
	{
		FlightData.Airspeed = FMath::Min(Settings.MaxThrustSpeed, FlightData.Airspeed + (Settings.AccelerationRate * FlightData.ThrottleInput * DeltaTime));
	}
	else
	{
		FlightData.Airspeed = FMath::Max(0.0f, FlightData.Airspeed - (Settings.AccelerationRate * (1.0f + Settings.DragCoefficient) * DeltaTime));
	}

	const bool bOldStall = FlightData.bIsStalling;

	if (FlightData.bVTOLMode)
	{
		FlightData.bIsAirborne = (FlightData.ThrottleInput > 0.0f || FlightData.Airspeed > 0.0f || FlightData.Altitude > 10.0f);
		FlightData.bIsStalling = false;
		FlightState = FlightData.bIsAirborne ? ESBFlightState::Airborne : ESBFlightState::Parked;
	}
	else
	{
		if (FlightData.Airspeed >= Settings.StallSpeed)
		{
			FlightData.bIsAirborne = true;
			FlightData.bIsStalling = false;
			FlightState = ESBFlightState::Airborne;
		}
		else
		{
			if (FlightData.bIsAirborne || FlightData.Altitude > 50.0f)
			{
				FlightData.bIsStalling = true;
				FlightState = ESBFlightState::Stalling;
			}
			else
			{
				FlightData.bIsAirborne = false;
				FlightData.bIsStalling = false;
				FlightState = FlightData.Airspeed > 10.0f ? ESBFlightState::Taxiing : ESBFlightState::Parked;
			}
		}
	}

	if (FlightData.bIsStalling != bOldStall)
	{
		OnAircraftStallStateChanged.Broadcast(FlightData.bIsStalling);
	}

	SyncFlightTags();
}

void USBAircraftComponent::SyncFlightTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	AActor* PilotActor = CurrentPilot.Get();
	USBStateComponent* PilotState = PilotActor ? PilotActor->FindComponentByClass<USBStateComponent>() : nullptr;

	if (PilotState)
	{
		PilotState->RemoveTag(Tags.State_Movement_Flying_Airborne);
		PilotState->RemoveTag(Tags.State_Movement_Flying_Stalling);
	}

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Movement_Flying_Airborne);
		CachedStateComp->RemoveTag(Tags.State_Movement_Flying_Stalling);
	}

	if (FlightData.bIsStalling)
	{
		if (PilotState) PilotState->AddTag(Tags.State_Movement_Flying_Stalling);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Flying_Stalling);
	}
	else if (FlightData.bIsAirborne)
	{
		if (PilotState) PilotState->AddTag(Tags.State_Movement_Flying_Airborne);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Flying_Airborne);
	}
}
