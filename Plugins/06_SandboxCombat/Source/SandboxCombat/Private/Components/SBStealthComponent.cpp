#include "Components/SBStealthComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBStealthComponent::USBStealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBStealthComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
	SyncStateTags();
}

void USBStealthComponent::OnShutdown_Implementation()
{
	NoiseHistory.Empty();
}

float USBStealthComponent::GetEffectiveVisibility() const
{
	float Vis = Settings.BaseVisibility;
	if (bIsCrouched)
	{
		Vis *= Settings.CrouchVisibilityMultiplier;
	}
	if (bIsInShadows)
	{
		Vis *= Settings.ShadowVisibilityMultiplier;
	}
	return Vis;
}

ESBStealthState USBStealthComponent::UpdateDetection(float BaseExposure, float DeltaTime)
{
	float EffVis = GetEffectiveVisibility();
	if (BaseExposure > 0.0f)
	{
		CurrentAlertPercent += (BaseExposure * EffVis * Settings.AlertBuildRate * DeltaTime);
	}
	else
	{
		CurrentAlertPercent -= (Settings.AlertDecayRate * DeltaTime);
	}

	CurrentAlertPercent = FMath::Clamp(CurrentAlertPercent, 0.0f, 1.0f);

	ESBStealthState NewState = ESBStealthState::Hidden;
	if (CurrentAlertPercent >= 1.0f)
	{
		NewState = ESBStealthState::Detected;
	}
	else if (CurrentAlertPercent > 0.0f)
	{
		NewState = ESBStealthState::Suspicious;
	}
	else
	{
		NewState = ESBStealthState::Hidden;
	}

	if (NewState != CurrentStealthState)
	{
		CurrentStealthState = NewState;
		SyncStateTags();
		OnStealthStateChanged.Broadcast(CurrentStealthState, CurrentAlertPercent);
	}

	return CurrentStealthState;
}

FSBNoiseEvent USBStealthComponent::EmitNoise(float Radius, float Loudness)
{
	FSBNoiseEvent Event;
	Event.Location = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	Event.Radius = Radius;
	Event.Loudness = Loudness;
	Event.Instigator = GetOwner();

	NoiseHistory.Add(Event);
	OnNoiseEmitted.Broadcast(Event);
	return Event;
}

void USBStealthComponent::ResetStealth()
{
	CurrentAlertPercent = 0.0f;
	CurrentStealthState = ESBStealthState::Hidden;
	bIsCrouched = false;
	bIsInShadows = false;
	SyncStateTags();
	OnStealthStateChanged.Broadcast(CurrentStealthState, 0.0f);
}

void USBStealthComponent::SyncStateTags()
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
	CachedStateComp->RemoveTag(Tags.State_Combat_Stealth_Hidden);
	CachedStateComp->RemoveTag(Tags.State_Combat_Stealth_Suspicious);
	CachedStateComp->RemoveTag(Tags.State_Combat_Stealth_Detected);

	switch (CurrentStealthState)
	{
	case ESBStealthState::Hidden:
		CachedStateComp->AddTag(Tags.State_Combat_Stealth_Hidden);
		break;
	case ESBStealthState::Suspicious:
		CachedStateComp->AddTag(Tags.State_Combat_Stealth_Suspicious);
		break;
	case ESBStealthState::Detected:
		CachedStateComp->AddTag(Tags.State_Combat_Stealth_Detected);
		break;
	}
}
