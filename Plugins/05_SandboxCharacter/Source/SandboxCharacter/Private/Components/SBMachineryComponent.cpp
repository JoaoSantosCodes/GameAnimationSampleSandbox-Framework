// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBMachineryComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBMachineryComponent::USBMachineryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBMachineryComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBMachineryComponent::OnShutdown_Implementation()
{
	if (CurrentOperator.IsValid())
	{
		ExitMachinery(CurrentOperator.Get());
	}
}

bool USBMachineryComponent::EnterMachinery(AActor* InOperator)
{
	if (!InOperator || CurrentOperator.IsValid())
	{
		return false;
	}

	CurrentOperator = InOperator;
	StartHydraulicPump(true);
	MachineryState = ESBMachineryState::Idling;

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Vehicle_Machinery);
	}

	if (USBStateComponent* OpState = InOperator->FindComponentByClass<USBStateComponent>())
	{
		OpState->AddTag(Tags.State_Movement_Machinery);
	}

	SyncMachineryTags();
	OnMachineryOperatorChanged.Broadcast(InOperator);
	OnMachineryStateChanged.Broadcast(MachineryState);
	return true;
}

bool USBMachineryComponent::ExitMachinery(AActor* InOperator)
{
	if (!InOperator || CurrentOperator.Get() != InOperator)
	{
		return false;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (USBStateComponent* OpState = InOperator->FindComponentByClass<USBStateComponent>())
	{
		OpState->RemoveTag(Tags.State_Movement_Machinery);
		OpState->RemoveTag(Tags.State_Movement_Machinery_Operating);
		OpState->RemoveTag(Tags.State_Movement_Machinery_Lifting);
		OpState->RemoveTag(Tags.State_Movement_Machinery_Excavating);
	}

	CurrentOperator = nullptr;
	BoomInput = 0.0f;
	ArmInput = 0.0f;
	BucketInput = 0.0f;
	SlewInput = 0.0f;
	WinchInput = 0.0f;
	StartHydraulicPump(false);
	MachineryState = ESBMachineryState::Parked;

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Movement_Machinery_Operating);
		CachedStateComp->RemoveTag(Tags.State_Movement_Machinery_Lifting);
		CachedStateComp->RemoveTag(Tags.State_Movement_Machinery_Excavating);
	}

	OnMachineryOperatorChanged.Broadcast(nullptr);
	OnMachineryStateChanged.Broadcast(MachineryState);
	return true;
}

void USBMachineryComponent::StartHydraulicPump(bool bStart)
{
	HydraulicData.bEngineRunning = bStart;
	HydraulicData.PumpRPM = bStart ? 1800.0f : 0.0f;
	if (!bStart)
	{
		HydraulicData.SystemPressure = 0.0f;
	}
	OnMachineryPressureChanged.Broadcast(HydraulicData.SystemPressure, HydraulicData.MaxSystemPressure);
}

void USBMachineryComponent::SetBoomInput(float Input)
{
	BoomInput = FMath::Clamp(Input, -1.0f, 1.0f);
}

void USBMachineryComponent::SetArmInput(float Input)
{
	ArmInput = FMath::Clamp(Input, -1.0f, 1.0f);
}

void USBMachineryComponent::SetBucketInput(float Input)
{
	BucketInput = FMath::Clamp(Input, -1.0f, 1.0f);
}

void USBMachineryComponent::SetSlewInput(float Input)
{
	SlewInput = FMath::Clamp(Input, -1.0f, 1.0f);
}

void USBMachineryComponent::SetWinchInput(float Input)
{
	WinchInput = FMath::Clamp(Input, -1.0f, 1.0f);
}

void USBMachineryComponent::SetOutriggersDeployed(bool bDeploy)
{
	HydraulicData.bOutriggersDeployed = bDeploy;
}

bool USBMachineryComponent::AttachPayload(float PayloadMass)
{
	if (!HydraulicData.bEngineRunning || PayloadMass > Settings.MaxLiftCapacity)
	{
		return false;
	}

	HydraulicData.LiftedPayloadMass = PayloadMass;
	MachineryState = ESBMachineryState::Lifting;
	SyncMachineryTags();
	OnMachineryStateChanged.Broadcast(MachineryState);
	return true;
}

void USBMachineryComponent::DetachPayload()
{
	HydraulicData.LiftedPayloadMass = 0.0f;
	MachineryState = CurrentOperator.IsValid() ? ESBMachineryState::Idling : ESBMachineryState::Parked;
	SyncMachineryTags();
	OnMachineryStateChanged.Broadcast(MachineryState);
}

bool USBMachineryComponent::TriggerExcavateAction()
{
	if (!HydraulicData.bEngineRunning || HydraulicData.SystemPressure < 100.0f)
	{
		return false;
	}

	MachineryState = ESBMachineryState::Excavating;
	SyncMachineryTags();
	OnMachineryStateChanged.Broadcast(MachineryState);
	return true;
}

void USBMachineryComponent::UpdateHydraulicPhysics(float DeltaTime)
{
	if (!HydraulicData.bEngineRunning)
	{
		HydraulicData.SystemPressure = 0.0f;
		MachineryState = ESBMachineryState::Parked;
		SyncMachineryTags();
		return;
	}

	HydraulicData.SystemPressure = FMath::Min(HydraulicData.MaxSystemPressure, HydraulicData.SystemPressure + (Settings.HydraulicBuildRate * DeltaTime));
	OnMachineryPressureChanged.Broadcast(HydraulicData.SystemPressure, HydraulicData.MaxSystemPressure);

	if (HydraulicData.SystemPressure >= 50.0f)
	{
		bool bActuating = false;

		if (FMath::Abs(BoomInput) > KINDA_SMALL_NUMBER)
		{
			HydraulicData.BoomAngle = FMath::Clamp(HydraulicData.BoomAngle + BoomInput * Settings.BoomSlewSpeed * DeltaTime, -20.0f, 80.0f);
			bActuating = true;
		}

		if (FMath::Abs(ArmInput) > KINDA_SMALL_NUMBER)
		{
			HydraulicData.ArmAngle = FMath::Clamp(HydraulicData.ArmAngle + ArmInput * Settings.BoomSlewSpeed * DeltaTime, 0.0f, 140.0f);
			bActuating = true;
		}

		if (FMath::Abs(BucketInput) > KINDA_SMALL_NUMBER)
		{
			HydraulicData.BucketAngle = FMath::Clamp(HydraulicData.BucketAngle + BucketInput * Settings.BoomSlewSpeed * DeltaTime, -45.0f, 90.0f);
			bActuating = true;
		}

		if (FMath::Abs(SlewInput) > KINDA_SMALL_NUMBER)
		{
			HydraulicData.CabinSlewAngle = FRotator::ClampAxis(HydraulicData.CabinSlewAngle + SlewInput * Settings.BoomSlewSpeed * DeltaTime);
			bActuating = true;
		}

		if (FMath::Abs(WinchInput) > KINDA_SMALL_NUMBER)
		{
			HydraulicData.CableLength = FMath::Max(1.0f, HydraulicData.CableLength + WinchInput * Settings.WinchSpeed * DeltaTime);
			bActuating = true;
		}

		if (HydraulicData.LiftedPayloadMass > 0.0f)
		{
			MachineryState = ESBMachineryState::Lifting;
		}
		else if (MachineryState == ESBMachineryState::Excavating)
		{
			// Keep excavating state
		}
		else if (bActuating)
		{
			MachineryState = ESBMachineryState::Operating;
		}
		else
		{
			MachineryState = CurrentOperator.IsValid() ? ESBMachineryState::Idling : ESBMachineryState::Parked;
		}
	}

	SyncMachineryTags();
}

void USBMachineryComponent::SyncMachineryTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	AActor* OpActor = CurrentOperator.Get();
	USBStateComponent* OpState = OpActor ? OpActor->FindComponentByClass<USBStateComponent>() : nullptr;

	if (OpState)
	{
		OpState->RemoveTag(Tags.State_Movement_Machinery_Operating);
		OpState->RemoveTag(Tags.State_Movement_Machinery_Lifting);
		OpState->RemoveTag(Tags.State_Movement_Machinery_Excavating);
	}

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Movement_Machinery_Operating);
		CachedStateComp->RemoveTag(Tags.State_Movement_Machinery_Lifting);
		CachedStateComp->RemoveTag(Tags.State_Movement_Machinery_Excavating);
	}

	if (MachineryState == ESBMachineryState::Lifting)
	{
		if (OpState) OpState->AddTag(Tags.State_Movement_Machinery_Lifting);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Machinery_Lifting);
	}
	else if (MachineryState == ESBMachineryState::Excavating)
	{
		if (OpState) OpState->AddTag(Tags.State_Movement_Machinery_Excavating);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Machinery_Excavating);
	}
	else if (MachineryState == ESBMachineryState::Operating)
	{
		if (OpState) OpState->AddTag(Tags.State_Movement_Machinery_Operating);
		if (CachedStateComp.IsValid()) CachedStateComp->AddTag(Tags.State_Movement_Machinery_Operating);
	}
}
