#pragma once

#include "CoreMinimal.h"
#include "SBThermalTypes.generated.h"

UENUM(BlueprintType)
enum class ESBThermalComfortState : uint8
{
	Freezing                UMETA(DisplayName = "Freezing"),
	Cold                    UMETA(DisplayName = "Cold"),
	Comfortable             UMETA(DisplayName = "Comfortable"),
	Warm                    UMETA(DisplayName = "Warm"),
	Overheating             UMETA(DisplayName = "Overheating"),
	CriticalHypothermia     UMETA(DisplayName = "CriticalHypothermia"),
	CriticalHeatstroke      UMETA(DisplayName = "CriticalHeatstroke")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBThermalRegulationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float CoreTemperature = 37.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float AmbientTemperature = 22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float ThermalInsulationCold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float ThermalInsulationHeat = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float WetnessLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float WindChill = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float NearbyHeatSource = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float HeatTransferRate = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	ESBThermalComfortState ComfortState = ESBThermalComfortState::Comfortable;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBThermalThresholdSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float HypothermiaThreshold = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float ColdThreshold = 36.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float WarmThreshold = 37.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float HeatstrokeThreshold = 39.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float FreezingDamagePerSec = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thermal")
	float HeatstrokeDamagePerSec = 2.0f;
};
