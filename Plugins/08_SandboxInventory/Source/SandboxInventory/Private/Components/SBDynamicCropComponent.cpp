#include "Components/SBDynamicCropComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBDynamicCropComponent::USBDynamicCropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetupCropPlot(0.5f, 1.0f);
}

void USBDynamicCropComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBDynamicCropComponent::SetupCropPlot(float InitialMoisture, float InitialFertility)
{
	CropData.Species = FSBPlantSpeciesData();
	CropData.Stage = ESBCropGrowthStage::Unplanted;
	CropData.GrowthProgress = 0.0f;
	CropData.SoilMoisture = FMath::Clamp(InitialMoisture, 0.0f, 1.0f);
	CropData.SoilFertility = FMath::Clamp(InitialFertility, 0.1f, 3.0f);
	CropData.bIsFertilized = false;
	CropData.HarvestYield = 0;
	SyncTags();
}

bool USBDynamicCropComponent::PlantSeed(const FSBPlantSpeciesData& Species)
{
	if (CropData.Stage != ESBCropGrowthStage::Unplanted && CropData.Stage != ESBCropGrowthStage::Withered)
	{
		return false;
	}

	ESBCropGrowthStage OldStage = CropData.Stage;
	CropData.Species = Species;
	CropData.Stage = ESBCropGrowthStage::Seeded;
	CropData.GrowthProgress = 0.0f;
	CropData.HarvestYield = 0;

	OnCropStageChanged.Broadcast(OldStage, ESBCropGrowthStage::Seeded);
	SyncTags();
	return true;
}

void USBDynamicCropComponent::WaterSoil(float WaterAmount)
{
	float OldMoisture = CropData.SoilMoisture;
	CropData.SoilMoisture = FMath::Clamp(CropData.SoilMoisture + FMath::Max(0.0f, WaterAmount), 0.0f, 1.0f);

	if (!FMath::IsNearlyEqual(OldMoisture, CropData.SoilMoisture))
	{
		OnSoilMoistureChanged.Broadcast(CropData.SoilMoisture);
	}
}

void USBDynamicCropComponent::ApplyFertilizer(float FertilityBoost)
{
	CropData.SoilFertility = FMath::Min(3.0f, CropData.SoilFertility + FMath::Max(0.0f, FertilityBoost));
	CropData.bIsFertilized = true;
	SyncTags();
}

bool USBDynamicCropComponent::HarvestCrop(int32& OutYield)
{
	if (CropData.Stage != ESBCropGrowthStage::Harvestable)
	{
		OutYield = 0;
		return false;
	}

	OutYield = FMath::Max(1, FMath::RoundToInt32(CropData.Species.BaseYield * CropData.SoilFertility));
	CropData.HarvestYield = OutYield;

	FName HarvestedSpecies = CropData.Species.SpeciesID;
	ESBCropGrowthStage OldStage = CropData.Stage;

	OnCropHarvested.Broadcast(HarvestedSpecies, OutYield);

	if (CropData.Species.bIsPerennial)
	{
		CropData.Stage = ESBCropGrowthStage::Vegetative;
		CropData.GrowthProgress = 0.4f;
		OnCropStageChanged.Broadcast(OldStage, ESBCropGrowthStage::Vegetative);
	}
	else
	{
		CropData.Stage = ESBCropGrowthStage::Unplanted;
		CropData.GrowthProgress = 0.0f;
		CropData.Species = FSBPlantSpeciesData();
		OnCropStageChanged.Broadcast(OldStage, ESBCropGrowthStage::Unplanted);
	}

	SyncTags();
	return true;
}

void USBDynamicCropComponent::SimulateCropTick(float DeltaTime, float SunExposure, float AmbientTemp)
{
	if (DeltaTime <= 0.0f || CropData.Stage == ESBCropGrowthStage::Unplanted || CropData.Stage == ESBCropGrowthStage::Withered)
	{
		return;
	}

	// Water consumption & evaporation
	float EvaporationRate = 0.01f * FMath::Max(0.0f, SunExposure) * (AmbientTemp > 30.0f ? 2.0f : 1.0f);
	float WaterConsumed = (CropData.Species.WaterConsumptionRate + EvaporationRate) * DeltaTime;

	float OldMoisture = CropData.SoilMoisture;
	CropData.SoilMoisture = FMath::Max(0.0f, CropData.SoilMoisture - WaterConsumed);

	if (!FMath::IsNearlyEqual(OldMoisture, CropData.SoilMoisture))
	{
		OnSoilMoistureChanged.Broadcast(CropData.SoilMoisture);
	}

	// Drought / Wither Check
	if (CropData.SoilMoisture <= 0.0f && AmbientTemp > 35.0f)
	{
		ESBCropGrowthStage OldStage = CropData.Stage;
		CropData.Stage = ESBCropGrowthStage::Withered;
		OnCropStageChanged.Broadcast(OldStage, ESBCropGrowthStage::Withered);
		OnCropWithered.Broadcast(CropData.Species.SpeciesID);
		SyncTags();
		return;
	}

	// Growth Advancement
	if (CropData.Stage != ESBCropGrowthStage::Harvestable)
	{
		float MoistureFactor = FMath::Max(0.2f, 1.0f - FMath::Abs(CropData.SoilMoisture - CropData.Species.OptimalMoisture));
		float TotalGrowthDuration = FMath::Max(1.0f, CropData.Species.GrowthDuration);
		float GrowthStep = (DeltaTime / TotalGrowthDuration) * MoistureFactor * CropData.SoilFertility;

		CropData.GrowthProgress = FMath::Clamp(CropData.GrowthProgress + GrowthStep, 0.0f, 1.0f);

		ESBCropGrowthStage OldStage = CropData.Stage;
		if (CropData.GrowthProgress >= 1.0f)
		{
			CropData.Stage = ESBCropGrowthStage::Harvestable;
		}
		else if (CropData.GrowthProgress >= 0.75f)
		{
			CropData.Stage = ESBCropGrowthStage::Flowering;
		}
		else if (CropData.GrowthProgress >= 0.4f)
		{
			CropData.Stage = ESBCropGrowthStage::Vegetative;
		}
		else if (CropData.GrowthProgress >= 0.15f)
		{
			CropData.Stage = ESBCropGrowthStage::Sprouting;
		}

		if (OldStage != CropData.Stage)
		{
			OnCropStageChanged.Broadcast(OldStage, CropData.Stage);
		}
	}

	SyncTags();
}

void USBDynamicCropComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Crop_Seeded);
	CachedStateComp->RemoveTag(Tags.State_Crop_Sprouting);
	CachedStateComp->RemoveTag(Tags.State_Crop_Growing);
	CachedStateComp->RemoveTag(Tags.State_Crop_Harvestable);
	CachedStateComp->RemoveTag(Tags.State_Crop_Withered);
	CachedStateComp->RemoveTag(Tags.State_Crop_Fertilized);

	switch (CropData.Stage)
	{
	case ESBCropGrowthStage::Seeded:
		CachedStateComp->AddTag(Tags.State_Crop_Seeded);
		break;
	case ESBCropGrowthStage::Sprouting:
		CachedStateComp->AddTag(Tags.State_Crop_Sprouting);
		break;
	case ESBCropGrowthStage::Vegetative:
	case ESBCropGrowthStage::Flowering:
		CachedStateComp->AddTag(Tags.State_Crop_Growing);
		break;
	case ESBCropGrowthStage::Harvestable:
		CachedStateComp->AddTag(Tags.State_Crop_Harvestable);
		break;
	case ESBCropGrowthStage::Withered:
		CachedStateComp->AddTag(Tags.State_Crop_Withered);
		break;
	default:
		break;
	}

	if (CropData.bIsFertilized)
	{
		CachedStateComp->AddTag(Tags.State_Crop_Fertilized);
	}
}
