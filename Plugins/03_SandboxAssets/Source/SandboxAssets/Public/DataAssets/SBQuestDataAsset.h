// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "DataAssets/SBPrimaryDataAsset.h"
#include "GameplayTagContainer.h"
#include "SBQuestDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FSBQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FGameplayTag ObjectiveTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	int32 RequiredCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText Description;
};

USTRUCT(BlueprintType)
struct FSBQuestReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TSoftObjectPtr<UObject> ItemDef = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	float Experience = 0.0f;
};

UCLASS(BlueprintType)
class SANDBOXASSETS_API USBQuestDataAsset : public USBPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText QuestDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FSBQuestObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FSBQuestReward> Rewards;
};
