#pragma once

#include "CoreMinimal.h"
#include "Behaviors/SBGameplayBehavior.h"
#include "GameplayTagContainer.h"
#include "SBAbility.generated.h"

UCLASS(Blueprintable, BlueprintType)
class SANDBOXCHARACTER_API USBAbility : public USBGameplayBehavior
{
	GENERATED_BODY()

public:
	USBAbility();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FGameplayTagContainer AbilityTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Cost")
	FGameplayTag ResourceTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Cost")
	float ResourceCost = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	float CooldownDuration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FGameplayTag CooldownTag;

	virtual void Enter_Implementation(const FSBBehaviorContext& Context) override;
	virtual void Exit_Implementation(const FSBBehaviorContext& Context) override;
};
