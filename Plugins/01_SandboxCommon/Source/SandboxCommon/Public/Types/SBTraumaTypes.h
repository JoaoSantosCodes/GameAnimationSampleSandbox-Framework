// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBTraumaTypes.generated.h"

UENUM(BlueprintType)
enum class ESBBodyLimb : uint8
{
	Head        UMETA(DisplayName = "Head"),
	Torso       UMETA(DisplayName = "Torso"),
	LeftArm     UMETA(DisplayName = "Left Arm"),
	RightArm    UMETA(DisplayName = "Right Arm"),
	LeftLeg     UMETA(DisplayName = "Left Leg"),
	RightLeg    UMETA(DisplayName = "Right Leg")
};

UENUM(BlueprintType)
enum class ESBBleedType : uint8
{
	None        UMETA(DisplayName = "None"),
	Venous      UMETA(DisplayName = "Venous"),
	Arterial    UMETA(DisplayName = "Arterial")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLimbTrauma
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	ESBBodyLimb Limb = ESBBodyLimb::Torso;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	float LimbHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	bool bIsFractured = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	bool bIsSplinted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	ESBBleedType BleedType = ESBBleedType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	bool bTourniquetApplied = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBTraumaSystemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	float BloodVolume = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	float MaxBloodVolume = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	float BloodRegenRate = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	TMap<ESBBodyLimb, FSBLimbTrauma> Limbs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trauma")
	bool bInHypovolemicShock = false;
};
