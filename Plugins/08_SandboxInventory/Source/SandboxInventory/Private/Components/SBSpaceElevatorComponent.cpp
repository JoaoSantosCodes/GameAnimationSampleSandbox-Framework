// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBSpaceElevatorComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBSpaceElevatorComponent::USBSpaceElevatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBSpaceElevatorComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBSpaceElevatorComponent::SetupSpaceElevator(int32 TotalPhases, float PowerRequiredMW)
{
	ElevatorData.MaxPhaseCount = TotalPhases;
	ElevatorData.PowerDemandMW = PowerRequiredMW;
	ElevatorData.CurrentPhaseIndex = 1;
	ElevatorData.State = ESBSpaceElevatorState::Idle;
	ElevatorData.PodAltitudeAlpha = 0.0f;
	ElevatorData.bHasPowerSupply = true;
	ElevatorData.Phases.Empty();
	SyncTags();
}

void USBSpaceElevatorComponent::ConfigurePhaseRequirement(int32 PhaseIndex, FName PhaseName, const TMap<FName, int32>& RequiredItems)
{
	for (FSBSpaceElevatorPhaseRequirement& Phase : ElevatorData.Phases)
	{
		if (Phase.PhaseIndex == PhaseIndex)
		{
			Phase.RequirementName = PhaseName;
			Phase.RequiredItems = RequiredItems;
			return;
		}
	}

	FSBSpaceElevatorPhaseRequirement NewPhase;
	NewPhase.PhaseIndex = PhaseIndex;
	NewPhase.RequirementName = PhaseName;
	NewPhase.RequiredItems = RequiredItems;
	NewPhase.bIsPhaseCompleted = false;
	ElevatorData.Phases.Add(NewPhase);
}

FSBSpaceElevatorPhaseRequirement* USBSpaceElevatorComponent::GetCurrentPhaseRequirement()
{
	for (FSBSpaceElevatorPhaseRequirement& Phase : ElevatorData.Phases)
	{
		if (Phase.PhaseIndex == ElevatorData.CurrentPhaseIndex)
		{
			return &Phase;
		}
	}
	return nullptr;
}

const FSBSpaceElevatorPhaseRequirement* USBSpaceElevatorComponent::GetCurrentPhaseRequirement() const
{
	for (const FSBSpaceElevatorPhaseRequirement& Phase : ElevatorData.Phases)
	{
		if (Phase.PhaseIndex == ElevatorData.CurrentPhaseIndex)
		{
			return &Phase;
		}
	}
	return nullptr;
}

int32 USBSpaceElevatorComponent::DepositPhaseItem(FName ItemId, int32 Quantity)
{
	if (ItemId.IsNone() || Quantity <= 0) return 0;
	if (ElevatorData.State != ESBSpaceElevatorState::Idle) return 0;

	FSBSpaceElevatorPhaseRequirement* Phase = GetCurrentPhaseRequirement();
	if (!Phase || Phase->bIsPhaseCompleted) return 0;

	const int32* RequiredCount = Phase->RequiredItems.Find(ItemId);
	if (!RequiredCount || *RequiredCount <= 0) return 0;

	int32 CurrentDeposited = Phase->DepositedItems.FindRef(ItemId);
	int32 Needed = FMath::Max(0, *RequiredCount - CurrentDeposited);
	int32 Added = FMath::Min(Quantity, Needed);

	if (Added > 0)
	{
		Phase->DepositedItems.FindOrAdd(ItemId) = CurrentDeposited + Added;
		OnSpaceElevatorItemDeposited.Broadcast(Phase->PhaseIndex, ItemId, Added);
	}

	return Added;
}

int32 USBSpaceElevatorComponent::GetDepositedItemCount(int32 PhaseIndex, FName ItemId) const
{
	for (const FSBSpaceElevatorPhaseRequirement& Phase : ElevatorData.Phases)
	{
		if (Phase.PhaseIndex == PhaseIndex)
		{
			return Phase.DepositedItems.FindRef(ItemId);
		}
	}
	return 0;
}

int32 USBSpaceElevatorComponent::GetRequiredItemCount(int32 PhaseIndex, FName ItemId) const
{
	for (const FSBSpaceElevatorPhaseRequirement& Phase : ElevatorData.Phases)
	{
		if (Phase.PhaseIndex == PhaseIndex)
		{
			return Phase.RequiredItems.FindRef(ItemId);
		}
	}
	return 0;
}

bool USBSpaceElevatorComponent::IsCurrentPhaseRequirementMet() const
{
	const FSBSpaceElevatorPhaseRequirement* Phase = GetCurrentPhaseRequirement();
	if (!Phase) return false;
	if (Phase->bIsPhaseCompleted) return true;

	if (Phase->RequiredItems.Num() == 0) return false;

	for (const auto& Pair : Phase->RequiredItems)
	{
		int32 Deposited = Phase->DepositedItems.FindRef(Pair.Key);
		if (Deposited < Pair.Value)
		{
			return false;
		}
	}

	return true;
}

bool USBSpaceElevatorComponent::LaunchOrbitalDelivery()
{
	if (ElevatorData.State != ESBSpaceElevatorState::Idle)
	{
		return false;
	}

	if (!IsCurrentPhaseRequirementMet())
	{
		return false;
	}

	if (!ElevatorData.bHasPowerSupply)
	{
		return false;
	}

	ElevatorData.State = ESBSpaceElevatorState::Ascending;
	ElevatorData.PodAltitudeAlpha = 0.0f;
	StateTimer = 0.0f;
	OnSpaceElevatorStateChanged.Broadcast(ElevatorData.CurrentPhaseIndex, ElevatorData.State);
	SyncTags();
	return true;
}

void USBSpaceElevatorComponent::SetPowerSupplied(bool bSupplied)
{
	ElevatorData.bHasPowerSupply = bSupplied;
}

void USBSpaceElevatorComponent::SimulateElevatorTick(float DeltaTime)
{
	switch (ElevatorData.State)
	{
	case ESBSpaceElevatorState::Ascending:
	{
		if (!ElevatorData.bHasPowerSupply)
		{
			// Stalled without power
			break;
		}

		ElevatorData.PodAltitudeAlpha = FMath::Clamp(ElevatorData.PodAltitudeAlpha + ElevatorData.PodAscentSpeed * DeltaTime, 0.0f, 1.0f);
		if (ElevatorData.PodAltitudeAlpha >= 1.0f)
		{
			ElevatorData.State = ESBSpaceElevatorState::DockedAtOrbitalStation;
			StateTimer = 0.0f;
			OnSpaceElevatorStateChanged.Broadcast(ElevatorData.CurrentPhaseIndex, ElevatorData.State);
		}
		break;
	}
	case ESBSpaceElevatorState::DockedAtOrbitalStation:
	{
		StateTimer += DeltaTime;
		if (StateTimer >= Settings.OrbitalStationWaitTime)
		{
			StateTimer = 0.0f;
			FSBSpaceElevatorPhaseRequirement* Phase = GetCurrentPhaseRequirement();
			if (Phase)
			{
				Phase->bIsPhaseCompleted = true;
				OnSpaceElevatorPhaseCompleted.Broadcast(Phase->PhaseIndex, Phase->RequirementName);
			}

			ElevatorData.State = ESBSpaceElevatorState::Descending;
			OnSpaceElevatorStateChanged.Broadcast(ElevatorData.CurrentPhaseIndex, ElevatorData.State);
		}
		break;
	}
	case ESBSpaceElevatorState::Descending:
	{
		ElevatorData.PodAltitudeAlpha = FMath::Clamp(ElevatorData.PodAltitudeAlpha - ElevatorData.PodDescentSpeed * DeltaTime, 0.0f, 1.0f);
		if (ElevatorData.PodAltitudeAlpha <= 0.0f)
		{
			ElevatorData.State = ESBSpaceElevatorState::Idle;
			ElevatorData.CurrentPhaseIndex = FMath::Min(ElevatorData.MaxPhaseCount, ElevatorData.CurrentPhaseIndex + 1);
			OnSpaceElevatorStateChanged.Broadcast(ElevatorData.CurrentPhaseIndex, ElevatorData.State);
		}
		break;
	}
	default:
		break;
	}

	SyncTags();
}

void USBSpaceElevatorComponent::SyncTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	CachedStateComp->RemoveTag(Tags.State_SpaceElevator_Idle);
	CachedStateComp->RemoveTag(Tags.State_SpaceElevator_Ascending);
	CachedStateComp->RemoveTag(Tags.State_SpaceElevator_Descending);
	CachedStateComp->RemoveTag(Tags.State_SpaceElevator_PhaseCompleted);
	CachedStateComp->RemoveTag(Tags.State_SpaceElevator_Delivering);

	switch (ElevatorData.State)
	{
	case ESBSpaceElevatorState::Idle:
		CachedStateComp->AddTag(Tags.State_SpaceElevator_Idle);
		break;
	case ESBSpaceElevatorState::Ascending:
		CachedStateComp->AddTag(Tags.State_SpaceElevator_Ascending);
		CachedStateComp->AddTag(Tags.State_SpaceElevator_Delivering);
		break;
	case ESBSpaceElevatorState::DockedAtOrbitalStation:
		CachedStateComp->AddTag(Tags.State_SpaceElevator_PhaseCompleted);
		break;
	case ESBSpaceElevatorState::Descending:
		CachedStateComp->AddTag(Tags.State_SpaceElevator_Descending);
		break;
	default:
		break;
	}
}
