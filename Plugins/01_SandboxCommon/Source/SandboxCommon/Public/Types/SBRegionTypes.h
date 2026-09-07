// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBRegionTypes.generated.h"

/**
 * Estrutura orientada a dados que define as regras e propriedades de uma Região / Zona de Perigo
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBRegionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FGameplayTag RegionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FText RegionDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	int32 DangerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	bool bIsSafeZone = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	bool bIsPvPAllowed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
	FGameplayTagContainer AppliedStateTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region|Hazard")
	float EnvironmentalDamagePerSecond = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region|Hazard")
	FGameplayTag HazardDamageTypeTag;
};
