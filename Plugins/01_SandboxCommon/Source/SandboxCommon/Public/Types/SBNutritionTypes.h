// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBNutritionTypes.generated.h"

UENUM(BlueprintType)
enum class ESBHungerLevel : uint8
{
	Satiated        UMETA(DisplayName = "Satiated"),
	Normal          UMETA(DisplayName = "Normal"),
	Hungry          UMETA(DisplayName = "Hungry"),
	Starving        UMETA(DisplayName = "Starving")
};

UENUM(BlueprintType)
enum class ESBHydrationLevel : uint8
{
	Hydrated            UMETA(DisplayName = "Hydrated"),
	Thirsty             UMETA(DisplayName = "Thirsty"),
	Dehydrated          UMETA(DisplayName = "Dehydrated"),
	CriticalDehydration UMETA(DisplayName = "CriticalDehydration")
};

UENUM(BlueprintType)
enum class ESBMetabolicActivityState : uint8
{
	Resting         UMETA(DisplayName = "Resting"),
	Walking         UMETA(DisplayName = "Walking"),
	Sprinting       UMETA(DisplayName = "Sprinting"),
	Combat          UMETA(DisplayName = "Combat"),
	Shivering       UMETA(DisplayName = "Shivering")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMicronutrientProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float VitaminA = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float VitaminB = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float VitaminC = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float VitaminD = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float Electrolytes = 100.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMetabolicNutritionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float Calories = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float MaxCalories = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float Hydration = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float MaxHydration = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float BaseBMR = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float HydrationLossRate = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	FSBMicronutrientProfile Nutrients;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	ESBHungerLevel HungerLevel = ESBHungerLevel::Satiated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	ESBHydrationLevel HydrationLevel = ESBHydrationLevel::Hydrated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	ESBMetabolicActivityState ActivityState = ESBMetabolicActivityState::Resting;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBConsumableNutritionItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float CalorieYield = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	float HydrationYield = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition")
	FSBMicronutrientProfile NutrientYield;
};
