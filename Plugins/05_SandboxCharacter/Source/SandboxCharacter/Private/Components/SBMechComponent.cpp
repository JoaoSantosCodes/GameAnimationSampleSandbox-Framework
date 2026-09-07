// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBMechComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBMechComponent::USBMechComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBMechComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBMechComponent::OnShutdown_Implementation()
{
	if (CurrentPilot.IsValid())
	{
		ExitMech(CurrentPilot.Get());
	}
}

bool USBMechComponent::EnterMech(AActor* InPilot)
{
	if (!InPilot || CurrentPilot.IsValid())
	{
		return false;
	}

	CurrentPilot = InPilot;
	OperationalData.bIsPowered = true;
	MechState = ESBMechState::Idle;

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Vehicle_Mech);
	}

	if (USBStateComponent* PilotState = InPilot->FindComponentByClass<USBStateComponent>())
	{
		PilotState->AddTag(Tags.State_Movement_Mech);
	}

	SyncMechTags();
	OnMechPilotChanged.Broadcast(InPilot);
	OnMechStateChanged.Broadcast(MechState);
	return true;
}

bool USBMechComponent::ExitMech(AActor* InPilot)
{
	if (!InPilot || CurrentPilot.Get() != InPilot)
	{
		return false;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (USBStateComponent* PilotState = InPilot->FindComponentByClass<USBStateComponent>())
	{
		PilotState->RemoveTag(Tags.State_Movement_Mech);
		PilotState->RemoveTag(Tags.State_Movement_Mech_Walking);
		PilotState->RemoveTag(Tags.State_Movement_Mech_JumpJets);
		PilotState->RemoveTag(Tags.State_Movement_Mech_Overheated);
	}

	CurrentPilot = nullptr;
	OperationalData.MoveInput = FVector::ZeroVector;
	OperationalData.CurrentSpeed = 0.0f;
	OperationalData.bJumpJetsActive = false;
	OperationalData.bIsPowered = false;
	MechState = ESBMechState::PoweredOff;

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Movement_Mech_Walking);
		CachedStateComp->RemoveTag(Tags.State_Movement_Mech_JumpJets);
		CachedStateComp->RemoveTag(Tags.State_Movement_Mech_Overheated);
	}

	OnMechPilotChanged.Broadcast(nullptr);
	OnMechStateChanged.Broadcast(MechState);
	return true;
}

void USBMechComponent::SetPowerState(bool bPowerOn)
{
	OperationalData.bIsPowered = bPowerOn;
	MechState = bPowerOn ? (CurrentPilot.IsValid() ? ESBMechState::Idle : ESBMechState::Idle) : ESBMechState::PoweredOff;
	SyncMechTags();
	OnMechStateChanged.Broadcast(MechState);
}

void USBMechComponent::SetMoveInput(const FVector& InMove)
{
	OperationalData.MoveInput = InMove.GetClampedToMaxSize(1.0f);
}

void USBMechComponent::ActivateJumpJets(bool bActive)
{
	if (bActive && (!OperationalData.bIsPowered || OperationalData.bIsOverheated || OperationalData.JumpJetFuel <= 0.0f))
	{
		OperationalData.bJumpJetsActive = false;
		return;
	}

	OperationalData.bJumpJetsActive = bActive;
}

bool USBMechComponent::TriggerDash()
{
	if (!OperationalData.bIsPowered || OperationalData.bIsOverheated)
	{
		return false;
	}

	OperationalData.CurrentSpeed = Settings.DashSpeed;
	OperationalData.CoreHeat = FMath::Min(OperationalData.MaxCoreHeat, OperationalData.CoreHeat + Settings.HeatGenerationRate * 0.5f);
	MechState = ESBMechState::Dashing;
	SyncMechTags();
	OnMechStateChanged.Broadcast(MechState);
	return true;
}

void USBMechComponent::UpdateMechPhysics(float DeltaTime)
{
	if (!OperationalData.bIsPowered)
	{
		OperationalData.CurrentSpeed = 0.0f;
		MechState = ESBMechState::PoweredOff;
		SyncMechTags();
		return;
	}

	if (OperationalData.CoreHeat >= Settings.OverheatThreshold && !OperationalData.bIsOverheated)
	{
		OperationalData.bIsOverheated = true;
		OperationalData.bJumpJetsActive = false;
		MechState = ESBMechState::Overheated;
		OnMechOverheatChanged.Broadcast(true);
	}
	else if (OperationalData.bIsOverheated && OperationalData.CoreHeat <= Settings.OverheatRecoveryThreshold)
	{
		OperationalData.bIsOverheated = false;
		MechState = ESBMechState::Idle;
		OnMechOverheatChanged.Broadcast(false);
	}

	if (OperationalData.bIsOverheated)
	{
		OperationalData.CoreHeat = FMath::Max(0.0f, OperationalData.CoreHeat - (Settings.HeatCoolingRate * DeltaTime));
		OperationalData.CurrentSpeed = 0.0f;
		MechState = ESBMechState::Overheated;
		SyncMechTags();
		return;
	}

	if (OperationalData.bJumpJetsActive)
	{
		OperationalData.JumpJetFuel = FMath::Max(0.0f, OperationalData.JumpJetFuel - (Settings.JumpJetFuelDrainRate * DeltaTime));
		OperationalData.CoreHeat = FMath::Min(OperationalData.MaxCoreHeat, OperationalData.CoreHeat + (Settings.HeatGenerationRate * DeltaTime));

		if (OperationalData.JumpJetFuel <= 0.0f)
		{
			OperationalData.bJumpJetsActive = false;
		}

		MechState = ESBMechState::JumpJets;
	}
	else
	{
		OperationalData.JumpJetFuel = FMath::Min(OperationalData.MaxJumpJetFuel, OperationalData.JumpJetFuel + (Settings.JumpJetFuelRechargeRate * DeltaTime));
		OperationalData.CoreHeat = FMath::Max(0.0f, OperationalData.CoreHeat - (Settings.HeatCoolingRate * DeltaTime));
	}

	if (MechState != ESBMechState::JumpJets && MechState != ESBMechState::Dashing)
	{
		if (!OperationalData.MoveInput.IsNearlyZero())
		{
			OperationalData.CurrentSpeed = Settings.MaxWalkSpeed * OperationalData.MoveInput.Size();
			MechState = ESBMechState::Walking;
		}
		else
		{
			OperationalData.CurrentSpeed = 0.0f;
			MechState = ESBMechState::Idle;
		}
	}

	SyncMechTags();
}

void USBMechComponent::SyncMechTags()
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
		PilotState->RemoveTag(Tags.State_Movement_Mech_Walking);
		PilotState->RemoveTag(Tags.State_Movement_Mech_JumpJets);
		PilotState->RemoveTag(Tags.State_Movement_Mech_Overheated);
	}

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Movement_Mech_Walking);
		CachedStateComp->RemoveTag(Tags.State_Movement_Mech_JumpJets);
		CachedStateComp->RemoveTag(Tags.State_Movement_Mech_Overheated);
	}

	if (OperationalData.bIsOverheated)
	{
		if (PilotState) PilotState->AddTag(Tags.State_Movement_Mech_Overheated);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Mech_Overheated);
	}
	else if (MechState == ESBMechState::JumpJets)
	{
		if (PilotState) PilotState->AddTag(Tags.State_Movement_Mech_JumpJets);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Mech_JumpJets);
	}
	else if (MechState == ESBMechState::Walking || MechState == ESBMechState::Dashing)
	{
		if (PilotState) PilotState->AddTag(Tags.State_Movement_Mech_Walking);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Mech_Walking);
	}
}
