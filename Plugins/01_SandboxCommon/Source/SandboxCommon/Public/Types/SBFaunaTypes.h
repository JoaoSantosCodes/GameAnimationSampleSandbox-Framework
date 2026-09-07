// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBFaunaTypes.generated.h"

UENUM(BlueprintType)
enum class ESBFaunaDomesticationState : uint8
{
	Wild            UMETA(DisplayName = "Wild"),
	Taming          UMETA(DisplayName = "Taming"),
	Domesticated    UMETA(DisplayName = "Domesticated"),
	Feral           UMETA(DisplayName = "Feral")
};

UENUM(BlueprintType)
enum class ESBFaunaReproductiveStage : uint8
{
	NonBreeding     UMETA(DisplayName = "NonBreeding"),
	Courtship       UMETA(DisplayName = "Courtship"),
	Pregnant        UMETA(DisplayName = "Pregnant"),
	Incubating      UMETA(DisplayName = "Incubating"),
	OffspringReady  UMETA(DisplayName = "OffspringReady")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCreatureGenetics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Genetics")
	float SpeedModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Genetics")
	float StaminaModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Genetics")
	float WeightCapacityModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Genetics")
	int32 Generation = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Genetics")
	int32 MutationCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Genetics")
	FColor CoatColor = FColor::White;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBDomesticationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Domestication")
	ESBFaunaDomesticationState DomesticationState = ESBFaunaDomesticationState::Wild;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Domestication")
	ESBFaunaReproductiveStage ReproductiveStage = ESBFaunaReproductiveStage::NonBreeding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Domestication")
	float TameProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Domestication")
	float AffectionLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Domestication")
	FName PreferredFoodID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Domestication")
	float PregnancyProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Domestication")
	float GestationDuration = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Domestication")
	FSBCreatureGenetics Genetics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fauna|Domestication")
	FSBCreatureGenetics MateGenetics;
};
