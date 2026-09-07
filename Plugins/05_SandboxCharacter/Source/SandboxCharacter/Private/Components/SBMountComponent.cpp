// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBMountComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBMountComponent::USBMountComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBMountComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedMountStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBMountComponent::OnShutdown_Implementation()
{
	if (IsMounted())
	{
		Dismount();
	}
}

bool USBMountComponent::Mount(AActor* InRider)
{
	if (!InRider || IsMounted())
	{
		return false;
	}

	RiderData.RiderActor = InRider;
	RiderData.MountActor = GetOwner();
	RiderData.MountState = ESBMountState::Mounted;
	RiderData.CurrentGait = ESBMountGait::Walk;
	RiderData.SaddleSocketName = Settings.DefaultSaddleSocket;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedMountStateComp.IsValid() && GetOwner())
	{
		CachedMountStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (CachedMountStateComp.IsValid())
	{
		CachedMountStateComp->AddTag(Tags.State_Movement_Mounted);
	}

	if (USBStateComponent* RiderState = InRider->FindComponentByClass<USBStateComponent>())
	{
		RiderState->AddTag(Tags.State_Movement_Mounted);
	}

	OnMountStateChanged.Broadcast(ESBMountState::Mounted, InRider);
	return true;
}

bool USBMountComponent::Dismount()
{
	if (!IsMounted())
	{
		return false;
	}

	AActor* OldRider = RiderData.RiderActor.Get();
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (CachedMountStateComp.IsValid())
	{
		CachedMountStateComp->RemoveTag(Tags.State_Movement_Mounted);
		CachedMountStateComp->RemoveTag(Tags.State_Movement_Galloping);
	}

	if (OldRider)
	{
		if (USBStateComponent* RiderState = OldRider->FindComponentByClass<USBStateComponent>())
		{
			RiderState->RemoveTag(Tags.State_Movement_Mounted);
		}
	}

	RiderData.RiderActor = nullptr;
	RiderData.MountState = ESBMountState::Unmounted;
	RiderData.CurrentGait = ESBMountGait::Walk;

	OnMountStateChanged.Broadcast(ESBMountState::Unmounted, OldRider);
	return true;
}

bool USBMountComponent::SetGait(ESBMountGait NewGait)
{
	if (!IsMounted())
	{
		return false;
	}

	RiderData.CurrentGait = NewGait;
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (CachedMountStateComp.IsValid())
	{
		if (NewGait == ESBMountGait::Gallop)
		{
			CachedMountStateComp->AddTag(Tags.State_Movement_Galloping);
		}
		else
		{
			CachedMountStateComp->RemoveTag(Tags.State_Movement_Galloping);
		}
	}

	OnMountGaitChanged.Broadcast(NewGait);
	return true;
}

float USBMountComponent::GetSpeedForGait(ESBMountGait Gait) const
{
	switch (Gait)
	{
	case ESBMountGait::Walk:
		return Settings.WalkSpeed;
	case ESBMountGait::Trot:
		return Settings.TrotSpeed;
	case ESBMountGait::Canter:
		return (Settings.TrotSpeed + Settings.GallopSpeed) * 0.5f;
	case ESBMountGait::Gallop:
		return Settings.GallopSpeed;
	default:
		return Settings.WalkSpeed;
	}
}
