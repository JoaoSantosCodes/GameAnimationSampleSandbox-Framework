#pragma once

#include "CoreMinimal.h"
#include "SBImmunityTypes.generated.h"

UENUM(BlueprintType)
enum class ESBInfectionStage : uint8
{
	Healthy         UMETA(DisplayName = "Healthy"),
	Incubating      UMETA(DisplayName = "Incubating"),
	Symptomatic     UMETA(DisplayName = "Symptomatic"),
	Severe          UMETA(DisplayName = "Severe"),
	Recovering      UMETA(DisplayName = "Recovering"),
	Immune          UMETA(DisplayName = "Immune")
};

UENUM(BlueprintType)
enum class ESBPathogenType : uint8
{
	Bacterial       UMETA(DisplayName = "Bacterial"),
	Viral           UMETA(DisplayName = "Viral"),
	Parasitic       UMETA(DisplayName = "Parasitic"),
	Fungal          UMETA(DisplayName = "Fungal")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBPathogenStrain
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	FName PathogenID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	ESBPathogenType Type = ESBPathogenType::Bacterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	float Virulence = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	float Severity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	float IncubationThreshold = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	float LethalThreshold = 100.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBActiveInfection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	FSBPathogenStrain Strain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	float PathogenLoad = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	float AntibodyCount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	float TreatmentEffectiveness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	ESBInfectionStage Stage = ESBInfectionStage::Healthy;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBImmuneSystemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	float BaseImmunityStrength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	float BodyFeverOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	TArray<FSBActiveInfection> ActiveInfections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Immunity")
	TArray<FName> AcquiredImmunities;
};
