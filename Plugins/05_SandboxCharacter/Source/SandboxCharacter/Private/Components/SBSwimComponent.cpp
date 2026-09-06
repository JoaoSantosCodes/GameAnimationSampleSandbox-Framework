#include "Components/SBSwimComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBSwimComponent::USBSwimComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBSwimComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBSwimComponent::OnShutdown_Implementation()
{
	ExitWater();
}

void USBSwimComponent::EnterWater(float InWaterSurfaceZ)
{
	WaterSurfaceZ = InWaterSurfaceZ;
	SwimState = ESBSwimState::SurfaceSwimming;
	SyncStateTags();
	OnSwimStateChanged.Broadcast(SwimState);
}

void USBSwimComponent::ExitWater()
{
	SwimState = ESBSwimState::None;
	OxygenData.bIsDrowning = false;
	SyncStateTags();
	OnSwimStateChanged.Broadcast(SwimState);
}

void USBSwimComponent::StartDiving()
{
	if (SwimState == ESBSwimState::None)
	{
		return;
	}

	SwimState = ESBSwimState::Diving;
	SyncStateTags();
	OnSwimStateChanged.Broadcast(SwimState);
}

void USBSwimComponent::SurfaceFromDive()
{
	if (SwimState == ESBSwimState::None)
	{
		return;
	}

	SwimState = ESBSwimState::SurfaceSwimming;
	OxygenData.bIsDrowning = false;
	SyncStateTags();
	OnSwimStateChanged.Broadcast(SwimState);
}

void USBSwimComponent::ConsumeOxygen(float DeltaTime)
{
	OxygenData.CurrentOxygen = FMath::Clamp(OxygenData.CurrentOxygen - (OxygenData.DepletionRatePerSec * DeltaTime), 0.0f, OxygenData.MaxOxygen);

	if (OxygenData.CurrentOxygen <= 0.0f && !OxygenData.bIsDrowning)
	{
		OxygenData.bIsDrowning = true;
		if (CachedStateComp.IsValid())
		{
			CachedStateComp->AddTag(FSBGameplayTags::Get().State_Status_Drowning);
		}
		OnDrowningStarted.Broadcast();
	}

	OnOxygenChanged.Broadcast(OxygenData.CurrentOxygen, OxygenData.MaxOxygen);
}

void USBSwimComponent::RecoverOxygen(float DeltaTime)
{
	OxygenData.CurrentOxygen = FMath::Clamp(OxygenData.CurrentOxygen + (OxygenData.RecoveryRatePerSec * DeltaTime), 0.0f, OxygenData.MaxOxygen);

	if (OxygenData.CurrentOxygen > 0.0f && OxygenData.bIsDrowning)
	{
		OxygenData.bIsDrowning = false;
		if (CachedStateComp.IsValid())
		{
			CachedStateComp->RemoveTag(FSBGameplayTags::Get().State_Status_Drowning);
		}
	}

	OnOxygenChanged.Broadcast(OxygenData.CurrentOxygen, OxygenData.MaxOxygen);
}

void USBSwimComponent::UpdateWaterLocomotion(float DeltaTime, float CurrentZ)
{
	if (SwimState == ESBSwimState::None)
	{
		return;
	}

	if (SwimState == ESBSwimState::Diving)
	{
		ConsumeOxygen(DeltaTime);
		if (CurrentZ >= (WaterSurfaceZ - Settings.WaterSurfaceTolerance))
		{
			SurfaceFromDive();
		}
	}
	else if (SwimState == ESBSwimState::SurfaceSwimming)
	{
		RecoverOxygen(DeltaTime);
		if (CurrentZ < (WaterSurfaceZ - Settings.WaterSurfaceTolerance))
		{
			StartDiving();
		}
	}
}

void USBSwimComponent::SyncStateTags()
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
	CachedStateComp->RemoveTag(Tags.State_Movement_Swimming);
	CachedStateComp->RemoveTag(Tags.State_Movement_Swimming_Surface);
	CachedStateComp->RemoveTag(Tags.State_Movement_Swimming_Diving);
	CachedStateComp->RemoveTag(Tags.State_Status_Drowning);

	switch (SwimState)
	{
	case ESBSwimState::SurfaceSwimming:
		CachedStateComp->AddTag(Tags.State_Movement_Swimming);
		CachedStateComp->AddTag(Tags.State_Movement_Swimming_Surface);
		break;
	case ESBSwimState::Diving:
		CachedStateComp->AddTag(Tags.State_Movement_Swimming);
		CachedStateComp->AddTag(Tags.State_Movement_Swimming_Diving);
		if (OxygenData.bIsDrowning)
		{
			CachedStateComp->AddTag(Tags.State_Status_Drowning);
		}
		break;
	case ESBSwimState::None:
	default:
		break;
	}
}
