#pragma once

#include "CoreMinimal.h"
#include "SBRadiationTypes.generated.h"

UENUM(BlueprintType)
enum class ESBRadiationSicknessStage : uint8
{
	None                    UMETA(DisplayName = "None"),
	MildExposure            UMETA(DisplayName = "MildExposure"),
	AcuteRadiationSickness  UMETA(DisplayName = "AcuteRadiationSickness"),
	CriticalLethalARS       UMETA(DisplayName = "CriticalLethalARS")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBRadiationEnvironmentData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radiation|Environment")
	float AmbientDoseRate_mSv_h = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radiation|Environment")
	float AirborneRadParticulatesPPM = 0.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBRadiationExposureData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radiation|Exposure")
	float AccumulatedDose_mSv = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radiation|Exposure")
	float CurrentDoseRate_mSv_h = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radiation|Exposure")
	float LeadShieldingFactor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radiation|Exposure")
	float GeigerClickFrequencyHz = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radiation|Exposure")
	ESBRadiationSicknessStage SicknessStage = ESBRadiationSicknessStage::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radiation|Exposure")
	bool bIsGeigerClicking = false;
};
