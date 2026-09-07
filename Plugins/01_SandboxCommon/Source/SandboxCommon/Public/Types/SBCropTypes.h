// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBCropTypes.generated.h"

UENUM(BlueprintType)
enum class ESBCropGrowthStage : uint8
{
	Unplanted       UMETA(DisplayName = "Unplanted"),
	Seeded          UMETA(DisplayName = "Seeded"),
	Sprouting       UMETA(DisplayName = "Sprouting"),
	Vegetative      UMETA(DisplayName = "Vegetative"),
	Flowering       UMETA(DisplayName = "Flowering"),
	Harvestable     UMETA(DisplayName = "Harvestable"),
	Withered        UMETA(DisplayName = "Withered")
};

UENUM(BlueprintType)
enum class ESBSoilHydrationLevel : uint8
{
	Parched         UMETA(DisplayName = "Parched"),
	Dry             UMETA(DisplayName = "Dry"),
	Moist           UMETA(DisplayName = "Moist"),
	Saturated       UMETA(DisplayName = "Saturated")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBPlantSpeciesData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName SpeciesID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float GrowthDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float OptimalMoisture = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float WaterConsumptionRate = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 BaseYield = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	bool bIsPerennial = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBDynamicCropData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FSBPlantSpeciesData Species;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ESBCropGrowthStage Stage = ESBCropGrowthStage::Unplanted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float GrowthProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float SoilMoisture = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float SoilFertility = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	bool bIsFertilized = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 HarvestYield = 0;
};
