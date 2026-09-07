// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBCoverComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBCoverComponent::USBCoverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBCoverComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBCoverComponent::OnShutdown_Implementation()
{
	ExitCover();
}

bool USBCoverComponent::EnterCover(const FSBCoverPoint& InCoverPoint)
{
	if (InCoverPoint.CoverType == ESBCoverType::None)
	{
		return false;
	}

	bIsInCover = true;
	CurrentCoverPoint = InCoverPoint;
	bIsPeeking = false;
	CurrentPeekEdge = ESBCoverEdge::None;

	SyncStateTags();
	OnCoverStateChanged.Broadcast(true, CurrentCoverPoint.CoverType);
	return true;
}

void USBCoverComponent::ExitCover()
{
	if (!bIsInCover)
	{
		return;
	}

	bIsInCover = false;
	bIsPeeking = false;
	CurrentPeekEdge = ESBCoverEdge::None;
	CurrentCoverPoint.CoverType = ESBCoverType::None;

	SyncStateTags();
	OnCoverStateChanged.Broadcast(false, ESBCoverType::None);
}

bool USBCoverComponent::StartPeeking(ESBCoverEdge InEdge)
{
	if (!bIsInCover || InEdge == ESBCoverEdge::None)
	{
		return false;
	}

	if (InEdge == ESBCoverEdge::Left && !CurrentCoverPoint.bHasLeftEdge)
	{
		return false;
	}
	if (InEdge == ESBCoverEdge::Right && !CurrentCoverPoint.bHasRightEdge)
	{
		return false;
	}
	if (InEdge == ESBCoverEdge::Top && !CurrentCoverPoint.bHasTopEdge)
	{
		return false;
	}

	bIsPeeking = true;
	CurrentPeekEdge = InEdge;

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(FSBGameplayTags::Get().State_Combat_Peeking);
	}

	OnPeekStateChanged.Broadcast(true, CurrentPeekEdge);
	return true;
}

void USBCoverComponent::StopPeeking()
{
	if (!bIsPeeking)
	{
		return;
	}

	bIsPeeking = false;
	CurrentPeekEdge = ESBCoverEdge::None;

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(FSBGameplayTags::Get().State_Combat_Peeking);
	}

	OnPeekStateChanged.Broadcast(false, ESBCoverEdge::None);
}

void USBCoverComponent::SyncStateTags()
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
	CachedStateComp->RemoveTag(Tags.State_Combat_InCover);
	CachedStateComp->RemoveTag(Tags.State_Combat_InCover_Low);
	CachedStateComp->RemoveTag(Tags.State_Combat_InCover_High);
	CachedStateComp->RemoveTag(Tags.State_Combat_Peeking);

	if (bIsInCover)
	{
		CachedStateComp->AddTag(Tags.State_Combat_InCover);
		if (CurrentCoverPoint.CoverType == ESBCoverType::LowCover)
		{
			CachedStateComp->AddTag(Tags.State_Combat_InCover_Low);
		}
		else if (CurrentCoverPoint.CoverType == ESBCoverType::HighCover)
		{
			CachedStateComp->AddTag(Tags.State_Combat_InCover_High);
		}

		if (bIsPeeking)
		{
			CachedStateComp->AddTag(Tags.State_Combat_Peeking);
		}
	}
}
