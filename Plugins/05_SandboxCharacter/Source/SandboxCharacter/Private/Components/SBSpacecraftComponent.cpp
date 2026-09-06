#include "Components/SBSpacecraftComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBSpacecraftComponent::USBSpacecraftComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBSpacecraftComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBSpacecraftComponent::OnShutdown_Implementation()
{
	if (CurrentPilot.IsValid())
	{
		ExitSpacecraft(CurrentPilot.Get());
	}
}

bool USBSpacecraftComponent::EnterSpacecraft(AActor* InPilot)
{
	if (!InPilot || CurrentPilot.IsValid())
	{
		return false;
	}

	CurrentPilot = InPilot;
	FlightState = ESBSpaceflightState::Drifting;

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Vehicle_Spacecraft);
	}

	if (USBStateComponent* PilotState = InPilot->FindComponentByClass<USBStateComponent>())
	{
		PilotState->AddTag(Tags.State_Movement_Spaceflight);
	}

	SyncFlightTags();
	OnSpacecraftPilotChanged.Broadcast(InPilot);
	OnSpaceflightStateChanged.Broadcast(FlightState);
	return true;
}

bool USBSpacecraftComponent::ExitSpacecraft(AActor* InPilot)
{
	if (!InPilot || CurrentPilot.Get() != InPilot)
	{
		return false;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (USBStateComponent* PilotState = InPilot->FindComponentByClass<USBStateComponent>())
	{
		PilotState->RemoveTag(Tags.State_Movement_Spaceflight);
		PilotState->RemoveTag(Tags.State_Movement_Spaceflight_Cruising);
		PilotState->RemoveTag(Tags.State_Movement_Spaceflight_FlightAssistOff);
		PilotState->RemoveTag(Tags.State_Movement_Spaceflight_Reentry);
	}

	CurrentPilot = nullptr;
	FlightData.TranslationInput = FVector::ZeroVector;
	FlightData.RotationInput = FVector::ZeroVector;
	FlightData.LinearVelocity = FVector::ZeroVector;
	FlightData.AngularVelocity = FVector::ZeroVector;
	FlightData.CurrentSpeed = 0.0f;
	FlightData.bInReentry = false;
	FlightState = ESBSpaceflightState::Docked;

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Movement_Spaceflight_Cruising);
		CachedStateComp->RemoveTag(Tags.State_Movement_Spaceflight_FlightAssistOff);
		CachedStateComp->RemoveTag(Tags.State_Movement_Spaceflight_Reentry);
	}

	OnSpacecraftPilotChanged.Broadcast(nullptr);
	OnSpaceflightStateChanged.Broadcast(FlightState);
	return true;
}

void USBSpacecraftComponent::SetTranslationInput(const FVector& InTranslation)
{
	FlightData.TranslationInput = InTranslation.GetClampedToMaxSize(1.0f);
}

void USBSpacecraftComponent::SetRotationInput(const FVector& InRotation)
{
	FlightData.RotationInput = InRotation.GetClampedToMaxSize(1.0f);
}

void USBSpacecraftComponent::SetFlightAssist(bool bEnable)
{
	FlightData.bFlightAssistActive = bEnable;
	SyncFlightTags();
	OnFlightAssistChanged.Broadcast(bEnable);
}

void USBSpacecraftComponent::SetBoostActive(bool bActive)
{
	FlightData.bBoostActive = bActive;
}

void USBSpacecraftComponent::SetAtmosphericReentry(bool bInReentry)
{
	FlightData.bInReentry = bInReentry;
	if (bInReentry)
	{
		FlightState = ESBSpaceflightState::Reentry;
	}
	SyncFlightTags();
}

void USBSpacecraftComponent::UpdateSpaceflightPhysics(float DeltaTime)
{
	const float CurrentMaxSpeed = FlightData.bBoostActive ? Settings.BoostMaxSpeed : Settings.MaxLinearSpeed;

	if (!FlightData.TranslationInput.IsNearlyZero())
	{
		const FVector Accel = FlightData.TranslationInput.GetClampedToMaxSize(1.0f) * Settings.LinearAcceleration * (FlightData.bBoostActive ? 2.0f : 1.0f);
		FlightData.LinearVelocity += Accel * DeltaTime;
		FlightData.LinearVelocity = FlightData.LinearVelocity.GetClampedToMaxSize(CurrentMaxSpeed);
	}
	else if (FlightData.bFlightAssistActive)
	{
		FlightData.LinearVelocity = FMath::VInterpTo(FlightData.LinearVelocity, FVector::ZeroVector, DeltaTime, Settings.InertiaDampingRate);
	}

	FlightData.CurrentSpeed = FlightData.LinearVelocity.Size();

	if (!FlightData.RotationInput.IsNearlyZero())
	{
		FlightData.AngularVelocity += FlightData.RotationInput.GetClampedToMaxSize(1.0f) * Settings.AngularAcceleration * DeltaTime;
	}
	else if (FlightData.bFlightAssistActive)
	{
		FlightData.AngularVelocity = FMath::VInterpTo(FlightData.AngularVelocity, FVector::ZeroVector, DeltaTime, Settings.InertiaDampingRate);
	}

	if (FlightData.bInReentry)
	{
		FlightState = ESBSpaceflightState::Reentry;
		FlightData.HeatShieldIntegrity = FMath::Max(0.0f, FlightData.HeatShieldIntegrity - (Settings.HeatDissipationRate * DeltaTime));
	}
	else if (FlightData.bBoostActive)
	{
		FlightState = ESBSpaceflightState::Boosting;
	}
	else if (!FlightData.TranslationInput.IsNearlyZero() || FlightData.CurrentSpeed > 50.0f)
	{
		FlightState = ESBSpaceflightState::Cruising;
	}
	else
	{
		FlightState = CurrentPilot.IsValid() ? ESBSpaceflightState::Drifting : ESBSpaceflightState::Docked;
	}

	SyncFlightTags();
}

void USBSpacecraftComponent::SyncFlightTags()
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
		PilotState->RemoveTag(Tags.State_Movement_Spaceflight_Cruising);
		PilotState->RemoveTag(Tags.State_Movement_Spaceflight_FlightAssistOff);
		PilotState->RemoveTag(Tags.State_Movement_Spaceflight_Reentry);
	}

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Movement_Spaceflight_Cruising);
		CachedStateComp->RemoveTag(Tags.State_Movement_Spaceflight_FlightAssistOff);
		CachedStateComp->RemoveTag(Tags.State_Movement_Spaceflight_Reentry);
	}

	if (FlightData.bInReentry)
	{
		if (PilotState) PilotState->AddTag(Tags.State_Movement_Spaceflight_Reentry);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Spaceflight_Reentry);
	}
	else if (FlightState == ESBSpaceflightState::Cruising || FlightState == ESBSpaceflightState::Boosting)
	{
		if (PilotState) PilotState->AddTag(Tags.State_Movement_Spaceflight_Cruising);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Spaceflight_Cruising);
	}

	if (!FlightData.bFlightAssistActive)
	{
		if (PilotState) PilotState->AddTag(Tags.State_Movement_Spaceflight_FlightAssistOff);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Spaceflight_FlightAssistOff);
	}
}
