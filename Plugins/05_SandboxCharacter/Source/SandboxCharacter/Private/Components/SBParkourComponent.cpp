// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBParkourComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBParkourComponent::USBParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBParkourComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBParkourComponent::OnShutdown_Implementation()
{
	CancelParkourAction();
}

FSBParkourObstacleData USBParkourComponent::DetectObstacle(const FVector& WallLocation, const FVector& WallNormal, float ObstacleHeight, float ObstacleDepth)
{
	FSBParkourObstacleData Data;
	Data.WallLocation = WallLocation;
	Data.WallNormal = WallNormal;
	Data.ObstacleHeight = ObstacleHeight;
	Data.ObstacleDepth = ObstacleDepth;
	Data.LedgeLocation = WallLocation + FVector(0.0f, 0.0f, ObstacleHeight);

	if (ObstacleHeight >= Settings.MinVaultHeight && ObstacleHeight <= Settings.MaxVaultHeight && ObstacleDepth <= Settings.MaxVaultDepth)
	{
		Data.RecommendedAction = ESBParkourActionType::Vault;
	}
	else if (ObstacleHeight > Settings.MaxVaultHeight && ObstacleHeight <= Settings.MaxMantleHeight)
	{
		Data.RecommendedAction = ESBParkourActionType::Mantle;
	}
	else
	{
		Data.RecommendedAction = ESBParkourActionType::None;
	}

	return Data;
}

bool USBParkourComponent::StartParkourAction(const FSBParkourObstacleData& InObstacleData)
{
	if (bIsPerformingParkour || InObstacleData.RecommendedAction == ESBParkourActionType::None)
	{
		return false;
	}

	bIsPerformingParkour = true;
	CurrentParkourAction = InObstacleData.RecommendedAction;
	CurrentObstacleData = InObstacleData;

	SyncStateTags();
	OnParkourActionStarted.Broadcast(CurrentParkourAction, CurrentObstacleData);
	return true;
}

void USBParkourComponent::CompleteParkourAction()
{
	if (!bIsPerformingParkour)
	{
		return;
	}

	ESBParkourActionType FinishedAction = CurrentParkourAction;
	bIsPerformingParkour = false;
	CurrentParkourAction = ESBParkourActionType::None;

	SyncStateTags();
	OnParkourActionCompleted.Broadcast(FinishedAction);
}

void USBParkourComponent::CancelParkourAction()
{
	if (!bIsPerformingParkour)
	{
		return;
	}

	bIsPerformingParkour = false;
	CurrentParkourAction = ESBParkourActionType::None;
	SyncStateTags();
}

void USBParkourComponent::SyncStateTags()
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
	CachedStateComp->RemoveTag(Tags.State_Movement_ParkourActive);
	CachedStateComp->RemoveTag(Tags.State_Movement_Vaulting);
	CachedStateComp->RemoveTag(Tags.State_Movement_Mantling);

	if (bIsPerformingParkour)
	{
		CachedStateComp->AddTag(Tags.State_Movement_ParkourActive);
		if (CurrentParkourAction == ESBParkourActionType::Vault)
		{
			CachedStateComp->AddTag(Tags.State_Movement_Vaulting);
		}
		else if (CurrentParkourAction == ESBParkourActionType::Mantle)
		{
			CachedStateComp->AddTag(Tags.State_Movement_Mantling);
		}
	}
}
