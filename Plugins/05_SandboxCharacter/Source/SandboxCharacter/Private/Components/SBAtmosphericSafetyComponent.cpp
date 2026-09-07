// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBAtmosphericSafetyComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBAtmosphericSafetyComponent::USBAtmosphericSafetyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetupAtmosphericSafety(100.0f, 100.0f);
}

void USBAtmosphericSafetyComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBAtmosphericSafetyComponent::SetupAtmosphericSafety(float InitialOxygen, float InitialFilter)
{
	SafetyData.BloodOxygenSaturation = 100.0f;
	SafetyData.ToxicityLevel = 0.0f;
	SafetyData.SuitOxygenReserve = InitialOxygen;
	SafetyData.MaxSuitOxygenReserve = 100.0f;
	SafetyData.FilterIntegrity = InitialFilter;
	SafetyData.SuitSealIntegrity = 100.0f;
	SafetyData.SuitState = ESBSuitPressurizationState::Unsealed;
	SafetyData.bIsHypoxic = false;
	SafetyData.bInToxicInhalation = false;
	SyncTags();
}

void USBAtmosphericSafetyComponent::ToggleSuitSeal(bool bSealSuit)
{
	if (bSealSuit)
	{
		SafetyData.SuitState = (SafetyData.SuitSealIntegrity > 20.0f) ? ESBSuitPressurizationState::Pressurized : ESBSuitPressurizationState::Breached;
	}
	else
	{
		SafetyData.SuitState = ESBSuitPressurizationState::Unsealed;
	}

	SyncTags();
}

void USBAtmosphericSafetyComponent::RefillOxygenReserve(float Amount)
{
	SafetyData.SuitOxygenReserve = FMath::Clamp(SafetyData.SuitOxygenReserve + Amount, 0.0f, SafetyData.MaxSuitOxygenReserve);
}

void USBAtmosphericSafetyComponent::ReplaceFilter(float NewFilterIntegrity)
{
	SafetyData.FilterIntegrity = FMath::Clamp(NewFilterIntegrity, 0.0f, 100.0f);
	if (SafetyData.FilterIntegrity > 0.0f && SafetyData.bInToxicInhalation)
	{
		SafetyData.bInToxicInhalation = false;
	}
	SyncTags();
}

void USBAtmosphericSafetyComponent::PatchSuitLeak(float SealRepairAmount)
{
	SafetyData.SuitSealIntegrity = FMath::Clamp(SafetyData.SuitSealIntegrity + SealRepairAmount, 0.0f, 100.0f);
	if (SafetyData.SuitState == ESBSuitPressurizationState::Breached && SafetyData.SuitSealIntegrity > 50.0f)
	{
		SafetyData.SuitState = ESBSuitPressurizationState::Pressurized;
	}
	SyncTags();
}

void USBAtmosphericSafetyComponent::SimulateAtmosphereTick(float DeltaTime, const FSBAtmosphereEnvironmentData& Environment)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	bool bIsSuitProvidingO2 = (SafetyData.SuitState == ESBSuitPressurizationState::Pressurized && SafetyData.SuitOxygenReserve > 0.0f);

	if (bIsSuitProvidingO2)
	{
		float O2ConsumptionRate = 0.5f;
		SafetyData.SuitOxygenReserve = FMath::Max(0.0f, SafetyData.SuitOxygenReserve - (O2ConsumptionRate * DeltaTime));
		SafetyData.BloodOxygenSaturation = FMath::Clamp(SafetyData.BloodOxygenSaturation + (5.0f * DeltaTime), 0.0f, 100.0f);

		if (SafetyData.SuitOxygenReserve <= 0.0f)
		{
			SafetyData.SuitState = ESBSuitPressurizationState::Compromised;
			OnSuitBreached.Broadcast();
		}
	}
	else
	{
		if (Environment.bIsVacuum || Environment.OxygenPercentage < 16.0f)
		{
			float DepletionRate = (Environment.bIsVacuum) ? 10.0f : (16.0f - Environment.OxygenPercentage) * 1.5f;
			SafetyData.BloodOxygenSaturation = FMath::Max(0.0f, SafetyData.BloodOxygenSaturation - (DepletionRate * DeltaTime));
		}
		else
		{
			SafetyData.BloodOxygenSaturation = FMath::Clamp(SafetyData.BloodOxygenSaturation + (5.0f * DeltaTime), 0.0f, 100.0f);
		}
	}

	bool bOldHypoxic = SafetyData.bIsHypoxic;
	SafetyData.bIsHypoxic = (SafetyData.BloodOxygenSaturation < 85.0f);
	if (bOldHypoxic != SafetyData.bIsHypoxic)
	{
		OnHypoxiaStateChanged.Broadcast(SafetyData.bIsHypoxic);
	}

	if (Environment.ToxicGasPPM > 50.0f)
	{
		if (SafetyData.FilterIntegrity > 0.0f)
		{
			float FilterWearRate = (Environment.ToxicGasPPM / 100.0f) * 1.0f;
			SafetyData.FilterIntegrity = FMath::Max(0.0f, SafetyData.FilterIntegrity - (FilterWearRate * DeltaTime));

			if (SafetyData.FilterIntegrity <= 0.0f)
			{
				OnFilterExhausted.Broadcast();
			}
		}

		// Avaliado sobre o estado APÓS o desgaste, e não como "else" do ramo acima: se o
		// filtro se esgota dentro deste mesmo tick, o personagem já passa a inalar o gás.
		// Com um "else", um tick que zerasse o filtro concederia um tick inteiro de imunidade.
		if (SafetyData.FilterIntegrity <= 0.0f)
		{
			SafetyData.bInToxicInhalation = true;
			SafetyData.ToxicityLevel = FMath::Clamp(SafetyData.ToxicityLevel + (2.0f * DeltaTime), 0.0f, 100.0f);
			OnToxicInhalationTriggered.Broadcast(SafetyData.ToxicityLevel);
		}
	}
	else
	{
		SafetyData.bInToxicInhalation = false;
		SafetyData.ToxicityLevel = FMath::Max(0.0f, SafetyData.ToxicityLevel - (1.0f * DeltaTime));
	}

	SyncTags();
}

void USBAtmosphericSafetyComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Atmosphere_Hazardous);
	CachedStateComp->RemoveTag(Tags.State_Atmosphere_Hypoxia);
	CachedStateComp->RemoveTag(Tags.State_Atmosphere_ToxicInhalation);
	CachedStateComp->RemoveTag(Tags.State_Atmosphere_SuitPressurized);
	CachedStateComp->RemoveTag(Tags.State_Atmosphere_FilterExhausted);

	if (SafetyData.bIsHypoxic)
	{
		CachedStateComp->AddTag(Tags.State_Atmosphere_Hypoxia);
		CachedStateComp->AddTag(Tags.State_Atmosphere_Hazardous);
	}

	if (SafetyData.bInToxicInhalation)
	{
		CachedStateComp->AddTag(Tags.State_Atmosphere_ToxicInhalation);
		CachedStateComp->AddTag(Tags.State_Atmosphere_Hazardous);
	}

	if (SafetyData.SuitState == ESBSuitPressurizationState::Pressurized)
	{
		CachedStateComp->AddTag(Tags.State_Atmosphere_SuitPressurized);
	}

	if (SafetyData.FilterIntegrity <= 0.0f)
	{
		CachedStateComp->AddTag(Tags.State_Atmosphere_FilterExhausted);
	}
}
