// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBPortalTypes.generated.h"

/**
 * Destino de teleporte configurável para um Portal
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBPortalDestination
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FGameplayTag TargetPortalTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FName TargetLevelName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float TransitionDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FGameplayTag RequiredKeyItemTag;
};

/**
 * Informações e estado de runtime de um Portal
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBPortalInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FGameplayTag PortalTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FText PortalDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	bool bIsOpen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	float CooldownDuration = 3.0f;
};
