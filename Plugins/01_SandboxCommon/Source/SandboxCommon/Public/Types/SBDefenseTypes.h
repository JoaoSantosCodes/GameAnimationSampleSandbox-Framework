#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBDefenseTypes.generated.h"

UENUM(BlueprintType)
enum class ESBBlockResult : uint8
{
	None UMETA(DisplayName = "None"),
	Blocked UMETA(DisplayName = "Blocked"),
	Parried UMETA(DisplayName = "Parried"),
	GuardBroken UMETA(DisplayName = "Guard Broken")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBDefenseSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	float BlockDamageReduction = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	float BlockStaminaCost = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	float ParryWindowDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	float CounterAttackWindowDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	float CounterAttackDamageMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	float StaggerDurationOnAttacker = 1.5f;
};
