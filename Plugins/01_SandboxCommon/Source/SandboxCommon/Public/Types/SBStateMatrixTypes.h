// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBStateMatrixTypes.generated.h"

/**
 * Estratégia de resolução de violação de matriz de estados
 */
UENUM(BlueprintType)
enum class ESBMatrixViolationAction : uint8
{
	WarnOnly UMETA(DisplayName = "Warn Only"),
	RejectTransition UMETA(DisplayName = "Reject Transition"),
	AutoResolvePrune UMETA(DisplayName = "Auto Resolve Prune")
};

/**
 * Regra de exclusão mútua entre tags
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBTagMutualExclusionRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateMatrix")
	FGameplayTag PrimaryTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateMatrix")
	FGameplayTagContainer IncompatibleTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateMatrix")
	ESBMatrixViolationAction ViolationAction = ESBMatrixViolationAction::AutoResolvePrune;

	FSBTagMutualExclusionRule() = default;

	FSBTagMutualExclusionRule(const FGameplayTag& InPrimary, const FGameplayTagContainer& InIncompatible, ESBMatrixViolationAction InAction = ESBMatrixViolationAction::AutoResolvePrune)
		: PrimaryTag(InPrimary)
		, IncompatibleTags(InIncompatible)
		, ViolationAction(InAction)
	{
	}
};

/**
 * Métricas e telemetria da matriz de estados
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBStateMatrixMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateMatrix")
	int32 TotalRulesRegistered = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateMatrix")
	int32 TotalEvaluations = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateMatrix")
	int32 TotalViolationsDetected = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateMatrix")
	int32 TotalTransitionsBlocked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateMatrix")
	int32 TotalTagsAutoPruned = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBOnMatrixViolationDetected, AActor*, TargetActor, FGameplayTag, AddedTag, FGameplayTag, ConflictingTag);
