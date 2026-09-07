// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBCombatFeedbackComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"
#include "Kismet/GameplayStatics.h"

USBCombatFeedbackComponent::USBCombatFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBCombatFeedbackComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBCombatFeedbackComponent::OnShutdown_Implementation()
{
	ResetHitStop();
	ResetSlomo();
}

void USBCombatFeedbackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsHitStopActive)
	{
		HitStopRemainingTime -= DeltaTime;
		if (HitStopRemainingTime <= 0.0f)
		{
			ResetHitStop();
		}
	}

	if (bIsSlomoActive)
	{
		SlomoRemainingTime -= DeltaTime;
		if (SlomoRemainingTime <= 0.0f)
		{
			ResetSlomo();
		}
	}
}

void USBCombatFeedbackComponent::ApplyHitStop(AActor* TargetActor, float Duration, float Dilation)
{
	bIsHitStopActive = true;
	HitStopRemainingTime = Duration;
	HitStopTargetActor = TargetActor;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (AActor* Owner = GetOwner())
	{
		Owner->CustomTimeDilation = Dilation;
		if (!CachedStateComp.IsValid())
		{
			CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
		}
		if (CachedStateComp.IsValid())
		{
			CachedStateComp->AddTag(Tags.State_Combat_HitStop);
		}
	}

	if (IsValid(TargetActor) && TargetActor != GetOwner())
	{
		TargetActor->CustomTimeDilation = Dilation;
		if (USBStateComponent* TargetState = TargetActor->FindComponentByClass<USBStateComponent>())
		{
			TargetState->AddTag(Tags.State_Combat_HitStop);
		}
	}

	OnHitStopTriggered.Broadcast(Duration);
}

void USBCombatFeedbackComponent::ApplyCombatFeedback(AActor* TargetActor, const FSBCombatFeedbackProfile& Profile)
{
	if (Profile.HitStop.Duration > 0.0f)
	{
		ApplyHitStop(TargetActor, Profile.HitStop.Duration, Profile.HitStop.TimeDilation);
	}

	if (Profile.Slomo.Duration > 0.0f)
	{
		TriggerSlomo(Profile.Slomo.TargetDilation, Profile.Slomo.Duration, Profile.Slomo.bGlobal);
	}
}

void USBCombatFeedbackComponent::TriggerSlomo(float TargetDilation, float Duration, bool bGlobal)
{
	bIsSlomoActive = true;
	SlomoRemainingTime = Duration;
	bIsSlomoGlobal = bGlobal;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (bGlobal)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TargetDilation);
	}
	else if (AActor* Owner = GetOwner())
	{
		Owner->CustomTimeDilation = TargetDilation;
	}

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Combat_Slomo);
	}

	OnSlomoTriggered.Broadcast(TargetDilation, Duration);
}

void USBCombatFeedbackComponent::ResetHitStop()
{
	if (!bIsHitStopActive)
	{
		return;
	}

	bIsHitStopActive = false;
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (AActor* Owner = GetOwner())
	{
		Owner->CustomTimeDilation = 1.0f;
		if (CachedStateComp.IsValid())
		{
			CachedStateComp->RemoveTag(Tags.State_Combat_HitStop);
		}
	}

	if (HitStopTargetActor.IsValid())
	{
		HitStopTargetActor->CustomTimeDilation = 1.0f;
		if (USBStateComponent* TargetState = HitStopTargetActor->FindComponentByClass<USBStateComponent>())
		{
			TargetState->RemoveTag(Tags.State_Combat_HitStop);
		}
		HitStopTargetActor = nullptr;
	}
}

void USBCombatFeedbackComponent::ResetSlomo()
{
	if (!bIsSlomoActive)
	{
		return;
	}

	bIsSlomoActive = false;
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (bIsSlomoGlobal)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	}
	else if (AActor* Owner = GetOwner())
	{
		Owner->CustomTimeDilation = 1.0f;
	}

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Combat_Slomo);
	}
}
