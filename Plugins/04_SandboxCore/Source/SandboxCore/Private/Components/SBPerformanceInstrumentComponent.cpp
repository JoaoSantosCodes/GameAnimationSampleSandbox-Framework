// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBPerformanceInstrumentComponent.h"
#include "Subsystems/SBPerformanceProfilerSubsystem.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBPerformanceInstrumentComponent::USBPerformanceInstrumentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBPerformanceInstrumentComponent::OnInitialize_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (USBPerformanceProfilerSubsystem* Subsystem = World->GetSubsystem<USBPerformanceProfilerSubsystem>())
		{
			Subsystem->SetBudgetThreshold(TrackedComponentName, BudgetThresholdUs);
			Subsystem->OnComponentBudgetExceeded.AddDynamic(this, &USBPerformanceInstrumentComponent::HandleBudgetExceeded);
		}
	}
	SyncTags();
}

void USBPerformanceInstrumentComponent::OnReady_Implementation()
{
	SyncTags();
}

void USBPerformanceInstrumentComponent::OnShutdown_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (USBPerformanceProfilerSubsystem* Subsystem = World->GetSubsystem<USBPerformanceProfilerSubsystem>())
		{
			Subsystem->OnComponentBudgetExceeded.RemoveDynamic(this, &USBPerformanceInstrumentComponent::HandleBudgetExceeded);
		}
	}

	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Profiler_Instrumented);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Profiler_BudgetExceeded);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Profiler_SamplingActive);
		}
	}
}

void USBPerformanceInstrumentComponent::RecordExecution(float DurationUs, int64 MemoryBytes)
{
	if (UWorld* World = GetWorld())
	{
		if (USBPerformanceProfilerSubsystem* Subsystem = World->GetSubsystem<USBPerformanceProfilerSubsystem>())
		{
			Subsystem->RecordComponentSample(TrackedComponentName, DurationUs, MemoryBytes);
			LocalExecutionCount++;
		}
	}
}

void USBPerformanceInstrumentComponent::HandleBudgetExceeded(FName InComponentName, float ActualDurationUs, float InBudgetThresholdUs)
{
	if (InComponentName == TrackedComponentName)
	{
		if (AActor* Owner = GetOwner())
		{
			if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
			{
				ISBStateComponentInterface::Execute_AddTag(StateComp, FSBGameplayTags::Get().State_Profiler_BudgetExceeded);
			}
		}
	}
}

void USBPerformanceInstrumentComponent::SyncTags()
{
	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Profiler_Instrumented);
			ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Profiler_SamplingActive);
		}
	}
}
