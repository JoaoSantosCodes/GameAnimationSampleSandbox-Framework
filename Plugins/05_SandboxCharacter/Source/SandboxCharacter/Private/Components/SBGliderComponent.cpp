// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBGliderComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBGliderComponent::USBGliderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBGliderComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBGliderComponent::OnShutdown_Implementation()
{
	RetractGlider();
}

bool USBGliderComponent::DeployGlider()
{
	if (IsGliding())
	{
		return false;
	}

	GliderState = ESBGliderState::Gliding;
	FlightData.bIsGliding = true;
	FlightData.CurrentFallSpeed = Settings.GlideFallSpeed;
	FlightData.CurrentForwardSpeed = Settings.GlideForwardSpeed;
	FlightData.CurrentPitchAngle = 0.0f;

	SyncStateTags();
	OnGliderStateChanged.Broadcast(GliderState);
	return true;
}

bool USBGliderComponent::RetractGlider()
{
	if (!IsGliding())
	{
		return false;
	}

	GliderState = ESBGliderState::Retracted;
	FlightData.bIsGliding = false;
	FlightData.CurrentFallSpeed = 0.0f;
	FlightData.CurrentForwardSpeed = 0.0f;
	FlightData.CurrentPitchAngle = 0.0f;

	SyncStateTags();
	OnGliderStateChanged.Broadcast(GliderState);
	return true;
}

bool USBGliderComponent::StartAerialDive()
{
	if (!IsGliding())
	{
		return false;
	}

	GliderState = ESBGliderState::Diving;
	FlightData.CurrentFallSpeed = Settings.DiveFallSpeed;
	FlightData.CurrentForwardSpeed = Settings.DiveForwardSpeed;

	SyncStateTags();
	OnGliderStateChanged.Broadcast(GliderState);
	return true;
}

bool USBGliderComponent::StopAerialDive()
{
	if (GliderState != ESBGliderState::Diving)
	{
		return false;
	}

	GliderState = ESBGliderState::Gliding;
	FlightData.CurrentFallSpeed = Settings.GlideFallSpeed;
	FlightData.CurrentForwardSpeed = Settings.GlideForwardSpeed;

	SyncStateTags();
	OnGliderStateChanged.Broadcast(GliderState);
	return true;
}

void USBGliderComponent::UpdateFlightPhysics(float DeltaTime, float PitchInput)
{
	if (!IsGliding())
	{
		return;
	}

	FlightData.CurrentPitchAngle = PitchInput;

	if (PitchInput > 0.5f && GliderState != ESBGliderState::Diving)
	{
		StartAerialDive();
	}
	else if (PitchInput <= 0.5f && GliderState == ESBGliderState::Diving)
	{
		StopAerialDive();
	}
}

void USBGliderComponent::SyncStateTags()
{
	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	CachedStateComp->RemoveTag(Tags.State_Movement_Gliding);
	CachedStateComp->RemoveTag(Tags.State_Movement_Gliding_Deploying);
	CachedStateComp->RemoveTag(Tags.State_Movement_Gliding_Diving);

	switch (GliderState)
	{
	case ESBGliderState::Gliding:
		CachedStateComp->AddTag(Tags.State_Movement_Gliding);
		break;
	case ESBGliderState::Diving:
		CachedStateComp->AddTag(Tags.State_Movement_Gliding);
		CachedStateComp->AddTag(Tags.State_Movement_Gliding_Diving);
		break;
	case ESBGliderState::Deploying:
		CachedStateComp->AddTag(Tags.State_Movement_Gliding_Deploying);
		break;
	case ESBGliderState::Retracted:
	default:
		break;
	}
}
