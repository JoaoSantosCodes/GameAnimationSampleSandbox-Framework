// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBComboTypes.generated.h"

UENUM(BlueprintType)
enum class ESBComboInputType : uint8
{
	LightAttack UMETA(DisplayName = "Light Attack"),
	HeavyAttack UMETA(DisplayName = "Heavy Attack"),
	SpecialAbility UMETA(DisplayName = "Special Ability"),
	Finisher UMETA(DisplayName = "Finisher")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBComboNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	int32 NodeId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	ESBComboInputType ExpectedInput = ESBComboInputType::LightAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TArray<int32> BranchTargetNodeIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool bIsFinisher = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBComboTree
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FGameplayTag ComboTreeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TArray<FSBComboNode> Nodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float MaxWindowDuration = 1.5f;
};
