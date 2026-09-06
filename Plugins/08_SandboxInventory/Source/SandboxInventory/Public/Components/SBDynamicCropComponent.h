#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBCropTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBDynamicCropComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBCropStageChanged, ESBCropGrowthStage, OldStage, ESBCropGrowthStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBCropHarvested, FName, SpeciesID, int32, Yield);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBCropWithered, FName, SpeciesID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBSoilMoistureChanged, float, NewMoisture);

/**
 * Componente de botânica dinâmica, ciclos de plantio, umidade do solo e colheita
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBDynamicCropComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBDynamicCropComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup & Planting
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Crop")
	void SetupCropPlot(float InitialMoisture = 0.5f, float InitialFertility = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Crop")
	bool PlantSeed(const FSBPlantSpeciesData& Species);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Crop")
	void WaterSoil(float WaterAmount);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Crop")
	void ApplyFertilizer(float FertilityBoost);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Crop")
	bool HarvestCrop(int32& OutYield);

	// Simulation Tick
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Crop")
	void SimulateCropTick(float DeltaTime, float SunExposure = 1.0f, float AmbientTemp = 22.0f);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Crop")
	FSBDynamicCropData GetCropData() const { return CropData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Crop")
	ESBCropGrowthStage GetGrowthStage() const { return CropData.Stage; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Crop")
	bool IsHarvestable() const { return CropData.Stage == ESBCropGrowthStage::Harvestable; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Crop")
	bool IsWithered() const { return CropData.Stage == ESBCropGrowthStage::Withered; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Crop")
	FSBCropStageChanged OnCropStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Crop")
	FSBCropHarvested OnCropHarvested;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Crop")
	FSBCropWithered OnCropWithered;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Crop")
	FSBSoilMoistureChanged OnSoilMoistureChanged;

private:
	void SyncTags();

	UPROPERTY()
	FSBDynamicCropData CropData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
