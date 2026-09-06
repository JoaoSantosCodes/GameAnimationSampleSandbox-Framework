#pragma once

#include "CoreMinimal.h"
#include "SBHerbologyTypes.generated.h"

UENUM(BlueprintType)
enum class ESBAfflictionType : uint8
{
	None                UMETA(DisplayName = "None"),
	MotorImpairment     UMETA(DisplayName = "MotorImpairment"),
	TissueDegradation   UMETA(DisplayName = "TissueDegradation"),
	CellularStrain      UMETA(DisplayName = "CellularStrain")
};

UENUM(BlueprintType)
enum class ESBNeutralizerType : uint8
{
	None                UMETA(DisplayName = "None"),
	HerbalBalm          UMETA(DisplayName = "HerbalBalm"),
	SynthesizedAntidote UMETA(DisplayName = "SynthesizedAntidote"),
	UniversalPanacea    UMETA(DisplayName = "UniversalPanacea")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAfflictionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affliction")
	ESBAfflictionType AfflictionType = ESBAfflictionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affliction")
	float Severity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affliction")
	float DurationRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affliction")
	float MotorImpairmentMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCharacterAfflictionState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affliction")
	TArray<FSBAfflictionData> ActiveAfflictions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affliction")
	float InoculationResistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affliction")
	bool bIsMotorImpaired = false;
};
