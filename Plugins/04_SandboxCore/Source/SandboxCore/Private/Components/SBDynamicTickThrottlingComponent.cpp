#include "Components/SBDynamicTickThrottlingComponent.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "Subsystems/SBDynamicTickManagerSubsystem.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"

USBDynamicTickThrottlingComponent::USBDynamicTickThrottlingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsFrustumVisible = true;
	SetupThrottling(FSBTickThrottlingSettings());
}

void USBDynamicTickThrottlingComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	}

	if (UWorld* World = GetWorld())
	{
		if (USBDynamicTickManagerSubsystem* Manager = World->GetSubsystem<USBDynamicTickManagerSubsystem>())
		{
			Manager->RegisterThrottledComponent(this);
		}
	}

	SyncTags();
}

void USBDynamicTickThrottlingComponent::OnShutdown_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (USBDynamicTickManagerSubsystem* Manager = World->GetSubsystem<USBDynamicTickManagerSubsystem>())
		{
			Manager->UnregisterThrottledComponent(this);
		}
	}
}

void USBDynamicTickThrottlingComponent::SetupThrottling(const FSBTickThrottlingSettings& InSettings)
{
	Settings = InSettings;
	ThrottlingState.CurrentLOD = ESBTickLODLevel::LOD0_HighPriority;
	ThrottlingState.AccumulatedDeltaTime = 0.0f;
	ThrottlingState.TimeSinceLastTick = 0.0f;
	ThrottlingState.DistanceToNearestViewer = 0.0f;
	ThrottlingState.bShouldTickThisFrame = true;
	ThrottlingState.TotalTicksExecuted = 0;
	bIsFrustumVisible = true;
	SyncTags();
}

void USBDynamicTickThrottlingComponent::UpdateDistanceToViewer(float Distance, bool bInFrustum)
{
	ThrottlingState.DistanceToNearestViewer = FMath::Max(0.0f, Distance);
	bIsFrustumVisible = bInFrustum;
	RecalculateLOD();
}

void USBDynamicTickThrottlingComponent::RecalculateLOD()
{
	ESBTickLODLevel PreviousLOD = ThrottlingState.CurrentLOD;
	ESBTickLODLevel NewLOD = ESBTickLODLevel::LOD0_HighPriority;

	if (ThrottlingState.DistanceToNearestViewer <= Settings.LOD0_MaxDistance)
	{
		NewLOD = ESBTickLODLevel::LOD0_HighPriority;
	}
	else if (ThrottlingState.DistanceToNearestViewer <= Settings.LOD1_MaxDistance)
	{
		NewLOD = ESBTickLODLevel::LOD1_MediumPriority;
	}
	else if (ThrottlingState.DistanceToNearestViewer <= Settings.LOD2_MaxDistance)
	{
		NewLOD = ESBTickLODLevel::LOD2_LowPriority;
	}
	else
	{
		NewLOD = ESBTickLODLevel::LOD3_BackgroundBatch;
	}

	if (NewLOD != PreviousLOD)
	{
		ThrottlingState.CurrentLOD = NewLOD;
		OnLODLevelChanged.Broadcast(NewLOD);
		SyncTags();
	}
}

bool USBDynamicTickThrottlingComponent::AdvanceTick(float DeltaTime, float& OutConsolidatedDeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		OutConsolidatedDeltaTime = 0.0f;
		return false;
	}

	ThrottlingState.AccumulatedDeltaTime += DeltaTime;
	ThrottlingState.TimeSinceLastTick += DeltaTime;

	float TargetInterval = 0.0f;
	switch (ThrottlingState.CurrentLOD)
	{
	case ESBTickLODLevel::LOD0_HighPriority:
		TargetInterval = 0.0f;
		break;
	case ESBTickLODLevel::LOD1_MediumPriority:
		TargetInterval = Settings.LOD1_Interval;
		break;
	case ESBTickLODLevel::LOD2_LowPriority:
		TargetInterval = Settings.LOD2_Interval;
		break;
	case ESBTickLODLevel::LOD3_BackgroundBatch:
		TargetInterval = Settings.LOD3_Interval;
		break;
	case ESBTickLODLevel::LOD_Suspended:
		TargetInterval = TNumericLimits<float>::Max();
		break;
	}

	if (TargetInterval <= 0.0f || ThrottlingState.TimeSinceLastTick >= TargetInterval)
	{
		OutConsolidatedDeltaTime = ThrottlingState.AccumulatedDeltaTime;
		ThrottlingState.AccumulatedDeltaTime = 0.0f;
		ThrottlingState.TimeSinceLastTick = 0.0f;
		ThrottlingState.bShouldTickThisFrame = true;
		ThrottlingState.TotalTicksExecuted++;
		return true;
	}

	OutConsolidatedDeltaTime = 0.0f;
	ThrottlingState.bShouldTickThisFrame = false;
	return false;
}

void USBDynamicTickThrottlingComponent::ForceLOD(ESBTickLODLevel NewLOD)
{
	if (ThrottlingState.CurrentLOD != NewLOD)
	{
		ThrottlingState.CurrentLOD = NewLOD;
		OnLODLevelChanged.Broadcast(NewLOD);
		SyncTags();
	}
}

void USBDynamicTickThrottlingComponent::SyncTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	ISBStateComponentInterface::Execute_RemoveTag(CachedStateComp.Get(), Tags.State_Throttling_LOD0);
	ISBStateComponentInterface::Execute_RemoveTag(CachedStateComp.Get(), Tags.State_Throttling_LOD1);
	ISBStateComponentInterface::Execute_RemoveTag(CachedStateComp.Get(), Tags.State_Throttling_LOD2);
	ISBStateComponentInterface::Execute_RemoveTag(CachedStateComp.Get(), Tags.State_Throttling_Background);
	ISBStateComponentInterface::Execute_RemoveTag(CachedStateComp.Get(), Tags.State_Throttling_Suspended);

	switch (ThrottlingState.CurrentLOD)
	{
	case ESBTickLODLevel::LOD0_HighPriority:
		ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Throttling_LOD0);
		break;
	case ESBTickLODLevel::LOD1_MediumPriority:
		ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Throttling_LOD1);
		break;
	case ESBTickLODLevel::LOD2_LowPriority:
		ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Throttling_LOD2);
		break;
	case ESBTickLODLevel::LOD3_BackgroundBatch:
		ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Throttling_Background);
		break;
	case ESBTickLODLevel::LOD_Suspended:
		ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Throttling_Suspended);
		break;
	}
}
