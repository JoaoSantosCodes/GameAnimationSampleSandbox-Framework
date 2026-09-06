#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBPoiseTypes.generated.h"

UENUM(BlueprintType)
enum class ESBHitReactionDirection : uint8
{
	Front UMETA(DisplayName = "Front"),
	Back UMETA(DisplayName = "Back"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class ESBHitReactionIntensity : uint8
{
	None UMETA(DisplayName = "None"),
	Light UMETA(DisplayName = "Light"),
	Heavy UMETA(DisplayName = "Heavy"),
	Knockdown UMETA(DisplayName = "Knockdown")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBPoiseSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise")
	float MaxPoise = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise")
	float PoiseRegenRate = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise")
	float PoiseRegenDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise")
	float StaggerDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poise")
	bool bHasSuperArmor = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBHitReactionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Poise")
	ESBHitReactionDirection Direction = ESBHitReactionDirection::Front;

	UPROPERTY(BlueprintReadOnly, Category = "Poise")
	ESBHitReactionIntensity Intensity = ESBHitReactionIntensity::Light;

	UPROPERTY(BlueprintReadOnly, Category = "Poise")
	float PoiseDamageApplied = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Poise")
	bool bPoiseBroken = false;

	UPROPERTY(BlueprintReadOnly, Category = "Poise")
	bool bAbsorbedBySuperArmor = false;

	UPROPERTY(BlueprintReadOnly, Category = "Poise")
	FGameplayTag ReactionTag;
};
