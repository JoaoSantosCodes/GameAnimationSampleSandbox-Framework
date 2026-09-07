// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBFaultResilientComponent.h"
#include "Subsystems/SBFaultToleranceSubsystem.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBFaultResilientComponent::USBFaultResilientComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBFaultResilientComponent::OnInitialize_Implementation()
{
	SyncTags();
}

void USBFaultResilientComponent::OnReady_Implementation()
{
	SyncTags();
}

void USBFaultResilientComponent::OnShutdown_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Fault_Resilient);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Fault_FallbackActive);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Fault_Degraded);
		}
	}
}

float USBFaultResilientComponent::SafeEvaluate(float IncomingValue, bool bPrimaryAvailable)
{
	UWorld* World = GetWorld();
	if (World)
	{
		if (USBFaultToleranceSubsystem* FaultSubsystem = World->GetSubsystem<USBFaultToleranceSubsystem>())
		{
			LastEvaluatedValue = FaultSubsystem->QuerySafeValue(MonitoredService, IncomingValue, bPrimaryAvailable);
			bInFallbackMode = (!bPrimaryAvailable || FaultSubsystem->GetServiceMode(MonitoredService) >= ESBFaultToleranceMode::Fallback);
			SyncTags();
			return LastEvaluatedValue;
		}
	}

	LastEvaluatedValue = bPrimaryAvailable ? IncomingValue : SafeDefaultValue;
	bInFallbackMode = !bPrimaryAvailable;
	SyncTags();
	return LastEvaluatedValue;
}

void USBFaultResilientComponent::SyncTags()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	if (!StateComp)
	{
		return;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Fault_Resilient);

	if (bInFallbackMode)
	{
		ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Fault_FallbackActive);
	}
	else
	{
		ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Fault_FallbackActive);
	}
}
