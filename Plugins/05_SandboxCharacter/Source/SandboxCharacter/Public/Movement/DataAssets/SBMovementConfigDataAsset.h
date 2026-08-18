#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBMovementConfigDataAsset.generated.h"

class USBMovementBehavior;
class USBMovementBehaviorDefinition;

USTRUCT(BlueprintType)
struct FSBMovementBehaviorConfigEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	TSubclassOf<USBMovementBehavior> BehaviorClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<USBMovementBehaviorDefinition> DefinitionAsset = nullptr;
};

USTRUCT(BlueprintType)
struct FSBStaminaConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float SprintCost = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float JumpCost = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float RegenRate = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float RegenDelay = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	float ExhaustionRecoveryThreshold = 30.0f;
};

USTRUCT(BlueprintType)
struct FSBAntiCheatConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AntiCheat")
	float SpeedTolerance = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AntiCheat")
	float BaseDistanceMargin = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AntiCheat")
	float WarpThreshold = 3000.0f;
};

UCLASS(BlueprintType)
class SANDBOXCHARACTER_API USBMovementConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	TArray<FSBMovementBehaviorConfigEntry> ConfiguredBehaviors;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	FSBStaminaConfig StaminaConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AntiCheat")
	FSBAntiCheatConfig AntiCheatConfig;
};
