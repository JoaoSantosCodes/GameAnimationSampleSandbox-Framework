#include "Components/SBThermalRegulationComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBThermalRegulationComponent::USBThermalRegulationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBThermalRegulationComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBThermalRegulationComponent::SetupThermalRegulation(float InitialCoreTemp, float InitialAmbientTemp)
{
	ThermalData.CoreTemperature = InitialCoreTemp;
	ThermalData.AmbientTemperature = InitialAmbientTemp;
	ThermalData.ThermalInsulationCold = 0.0f;
	ThermalData.ThermalInsulationHeat = 0.0f;
	ThermalData.WetnessLevel = 0.0f;
	ThermalData.WindChill = 0.0f;
	ThermalData.NearbyHeatSource = 0.0f;
	ThermalData.HeatTransferRate = 0.05f;
	ThermalData.ComfortState = ESBThermalComfortState::Comfortable;
	SyncTags();
}

void USBThermalRegulationComponent::SetAmbientTemperature(float NewAmbientTemp)
{
	ThermalData.AmbientTemperature = NewAmbientTemp;
}

void USBThermalRegulationComponent::SetThermalInsulation(float ColdInsulation, float HeatInsulation)
{
	ThermalData.ThermalInsulationCold = FMath::Clamp(ColdInsulation, 0.0f, 1.0f);
	ThermalData.ThermalInsulationHeat = FMath::Clamp(HeatInsulation, 0.0f, 1.0f);
}

void USBThermalRegulationComponent::SetWetnessLevel(float InWetness)
{
	ThermalData.WetnessLevel = FMath::Clamp(InWetness, 0.0f, 1.0f);
}

void USBThermalRegulationComponent::SetNearbyHeatSource(float AddedHeatTemp)
{
	ThermalData.NearbyHeatSource = FMath::Max(0.0f, AddedHeatTemp);
}

void USBThermalRegulationComponent::SetWindChill(float InWindChill)
{
	ThermalData.WindChill = FMath::Max(0.0f, InWindChill);
}

float USBThermalRegulationComponent::GetEffectiveAmbientTemperature() const
{
	return ThermalData.AmbientTemperature - ThermalData.WindChill + ThermalData.NearbyHeatSource;
}

void USBThermalRegulationComponent::SimulateThermalTick(float DeltaTime)
{
	float EffectiveAmbient = GetEffectiveAmbientTemperature();
	float DeltaT = EffectiveAmbient - ThermalData.CoreTemperature;

	if (DeltaT < 0.0f)
	{
		// Cooling down
		float ColdResistance = FMath::Clamp(1.0f - ThermalData.ThermalInsulationCold, 0.05f, 1.0f);
		float WetnessFactor = 1.0f + ThermalData.WetnessLevel * 1.5f;
		float Rate = ThermalData.HeatTransferRate * ColdResistance * WetnessFactor;
		ThermalData.CoreTemperature += DeltaT * Rate * DeltaTime;
	}
	else if (DeltaT > 0.0f)
	{
		// Warming up
		float HeatResistance = FMath::Clamp(1.0f - ThermalData.ThermalInsulationHeat, 0.05f, 1.0f);
		float Rate = ThermalData.HeatTransferRate * HeatResistance;
		ThermalData.CoreTemperature += DeltaT * Rate * DeltaTime;
	}

	// Homeostatic recovery if in comfortable ambient range (18 to 28°C)
	if (EffectiveAmbient >= 18.0f && EffectiveAmbient <= 28.0f)
	{
		ThermalData.CoreTemperature = FMath::FInterpTo(ThermalData.CoreTemperature, 37.0f, DeltaTime, 0.5f);
	}

	ESBThermalComfortState OldState = ThermalData.ComfortState;

	if (ThermalData.CoreTemperature < Settings.HypothermiaThreshold)
	{
		ThermalData.ComfortState = ESBThermalComfortState::CriticalHypothermia;
	}
	else if (ThermalData.CoreTemperature < Settings.ColdThreshold)
	{
		if (ThermalData.CoreTemperature < Settings.HypothermiaThreshold + 0.6f)
		{
			ThermalData.ComfortState = ESBThermalComfortState::Freezing;
		}
		else
		{
			ThermalData.ComfortState = ESBThermalComfortState::Cold;
		}
	}
	else if (ThermalData.CoreTemperature > Settings.HeatstrokeThreshold)
	{
		ThermalData.ComfortState = ESBThermalComfortState::CriticalHeatstroke;
	}
	else if (ThermalData.CoreTemperature > Settings.WarmThreshold)
	{
		if (ThermalData.CoreTemperature > Settings.HeatstrokeThreshold - 0.6f)
		{
			ThermalData.ComfortState = ESBThermalComfortState::Overheating;
		}
		else
		{
			ThermalData.ComfortState = ESBThermalComfortState::Warm;
		}
	}
	else
	{
		ThermalData.ComfortState = ESBThermalComfortState::Comfortable;
	}

	if (OldState != ThermalData.ComfortState)
	{
		OnThermalComfortStateChanged.Broadcast(OldState, ThermalData.ComfortState, ThermalData.CoreTemperature);

		if (ThermalData.ComfortState == ESBThermalComfortState::CriticalHypothermia)
		{
			OnHypothermiaTriggered.Broadcast(ThermalData.CoreTemperature);
		}
		else if (ThermalData.ComfortState == ESBThermalComfortState::CriticalHeatstroke)
		{
			OnHeatstrokeTriggered.Broadcast(ThermalData.CoreTemperature);
		}
	}

	SyncTags();
}

void USBThermalRegulationComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Thermal_Comfortable);
	CachedStateComp->RemoveTag(Tags.State_Thermal_Cold);
	CachedStateComp->RemoveTag(Tags.State_Thermal_Freezing);
	CachedStateComp->RemoveTag(Tags.State_Thermal_Warm);
	CachedStateComp->RemoveTag(Tags.State_Thermal_Overheating);
	CachedStateComp->RemoveTag(Tags.State_Thermal_Hypothermia);
	CachedStateComp->RemoveTag(Tags.State_Thermal_Heatstroke);

	switch (ThermalData.ComfortState)
	{
	case ESBThermalComfortState::Comfortable:
		CachedStateComp->AddTag(Tags.State_Thermal_Comfortable);
		break;
	case ESBThermalComfortState::Cold:
		CachedStateComp->AddTag(Tags.State_Thermal_Cold);
		break;
	case ESBThermalComfortState::Freezing:
		CachedStateComp->AddTag(Tags.State_Thermal_Freezing);
		break;
	case ESBThermalComfortState::Warm:
		CachedStateComp->AddTag(Tags.State_Thermal_Warm);
		break;
	case ESBThermalComfortState::Overheating:
		CachedStateComp->AddTag(Tags.State_Thermal_Overheating);
		break;
	case ESBThermalComfortState::CriticalHypothermia:
		CachedStateComp->AddTag(Tags.State_Thermal_Hypothermia);
		break;
	case ESBThermalComfortState::CriticalHeatstroke:
		CachedStateComp->AddTag(Tags.State_Thermal_Heatstroke);
		break;
	default:
		break;
	}
}
