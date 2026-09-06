#include "Components/SBWatercraftComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBWatercraftComponent::USBWatercraftComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBWatercraftComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBWatercraftComponent::OnShutdown_Implementation()
{
	if (CurrentPilot.IsValid())
	{
		ExitWatercraft(CurrentPilot.Get());
	}
}

bool USBWatercraftComponent::EnterWatercraft(AActor* InPilot)
{
	if (!InPilot || CurrentPilot.IsValid())
	{
		return false;
	}

	CurrentPilot = InPilot;
	WatercraftState = NavigationData.bIsAnchored ? ESBWatercraftState::Anchored : ESBWatercraftState::Drifting;

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Vehicle_Watercraft);
	}

	if (USBStateComponent* PilotState = InPilot->FindComponentByClass<USBStateComponent>())
	{
		PilotState->AddTag(Tags.State_Movement_Sailing);
	}

	OnWatercraftPilotChanged.Broadcast(InPilot);
	OnWatercraftStateChanged.Broadcast(WatercraftState);
	return true;
}

bool USBWatercraftComponent::ExitWatercraft(AActor* InPilot)
{
	if (!InPilot || CurrentPilot.Get() != InPilot)
	{
		return false;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (USBStateComponent* PilotState = InPilot->FindComponentByClass<USBStateComponent>())
	{
		PilotState->RemoveTag(Tags.State_Movement_Sailing);
		PilotState->RemoveTag(Tags.State_Movement_Sailing_Cruising);
		PilotState->RemoveTag(Tags.State_Movement_Sailing_Anchored);
	}

	CurrentPilot = nullptr;
	NavigationData.ThrottleInput = 0.0f;
	NavigationData.CurrentSpeed = 0.0f;
	WatercraftState = NavigationData.bIsAnchored ? ESBWatercraftState::Anchored : ESBWatercraftState::Docked;

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Movement_Sailing_Cruising);
	}

	OnWatercraftPilotChanged.Broadcast(nullptr);
	OnWatercraftStateChanged.Broadcast(WatercraftState);
	return true;
}

void USBWatercraftComponent::SetWaterSurfaceLevel(float InWaterZ)
{
	NavigationData.WaterLevelZ = InWaterZ;
}

bool USBWatercraftComponent::DropAnchor()
{
	if (NavigationData.bIsAnchored)
	{
		return false;
	}

	NavigationData.bIsAnchored = true;
	NavigationData.CurrentSpeed = 0.0f;
	NavigationData.ThrottleInput = 0.0f;
	WatercraftState = ESBWatercraftState::Anchored;

	SyncSailingTags();
	OnWatercraftAnchorChanged.Broadcast(true);
	OnWatercraftStateChanged.Broadcast(WatercraftState);
	return true;
}

bool USBWatercraftComponent::RaiseAnchor()
{
	if (!NavigationData.bIsAnchored)
	{
		return false;
	}

	NavigationData.bIsAnchored = false;
	WatercraftState = CurrentPilot.IsValid() ? ESBWatercraftState::Drifting : ESBWatercraftState::Docked;

	SyncSailingTags();
	OnWatercraftAnchorChanged.Broadcast(false);
	OnWatercraftStateChanged.Broadcast(WatercraftState);
	return true;
}

void USBWatercraftComponent::SetThrottleInput(float InThrottle)
{
	NavigationData.ThrottleInput = FMath::Clamp(InThrottle, -1.0f, 1.0f);
}

void USBWatercraftComponent::SetRudderInput(float InRudder)
{
	NavigationData.RudderInput = FMath::Clamp(InRudder, -1.0f, 1.0f);
}

void USBWatercraftComponent::UpdateWatercraftPhysics(float DeltaTime, const FVector& CurrentActorLocation)
{
	NavigationData.BuoyancyDepth = FMath::Max(0.0f, NavigationData.WaterLevelZ - CurrentActorLocation.Z);

	if (NavigationData.bIsAnchored)
	{
		NavigationData.CurrentSpeed = 0.0f;
		SyncSailingTags();
		return;
	}

	if (NavigationData.ThrottleInput > 0.0f)
	{
		NavigationData.CurrentSpeed = FMath::Min(Settings.MaxForwardSpeed, NavigationData.CurrentSpeed + (Settings.AccelerationRate * NavigationData.ThrottleInput * DeltaTime));
		WatercraftState = ESBWatercraftState::Cruising;
	}
	else if (NavigationData.ThrottleInput < 0.0f)
	{
		NavigationData.CurrentSpeed = FMath::Max(-Settings.MaxReverseSpeed, NavigationData.CurrentSpeed + (Settings.AccelerationRate * NavigationData.ThrottleInput * DeltaTime));
		WatercraftState = ESBWatercraftState::Cruising;
	}
	else
	{
		if (NavigationData.CurrentSpeed > 0.0f)
		{
			NavigationData.CurrentSpeed = FMath::Max(0.0f, NavigationData.CurrentSpeed - (Settings.WaterResistance * DeltaTime));
		}
		else if (NavigationData.CurrentSpeed < 0.0f)
		{
			NavigationData.CurrentSpeed = FMath::Min(0.0f, NavigationData.CurrentSpeed + (Settings.WaterResistance * DeltaTime));
		}

		if (FMath::IsNearlyZero(NavigationData.CurrentSpeed, 1.0f))
		{
			WatercraftState = CurrentPilot.IsValid() ? ESBWatercraftState::Drifting : ESBWatercraftState::Docked;
		}
	}

	SyncSailingTags();
}

void USBWatercraftComponent::SyncSailingTags()
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
		PilotState->RemoveTag(Tags.State_Movement_Sailing_Cruising);
		PilotState->RemoveTag(Tags.State_Movement_Sailing_Anchored);
	}

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Movement_Sailing_Cruising);
		CachedStateComp->RemoveTag(Tags.State_Movement_Sailing_Anchored);
	}

	if (NavigationData.bIsAnchored)
	{
		if (PilotState) PilotState->AddTag(Tags.State_Movement_Sailing_Anchored);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Sailing_Anchored);
	}
	else if (WatercraftState == ESBWatercraftState::Cruising)
	{
		if (PilotState) PilotState->AddTag(Tags.State_Movement_Sailing_Cruising);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Sailing_Cruising);
	}
}
