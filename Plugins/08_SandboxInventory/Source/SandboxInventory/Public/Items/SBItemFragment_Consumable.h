#pragma once

#include "CoreMinimal.h"
#include "Items/SBItemFragment.h"
#include "GameplayTagContainer.h"
#include "SBItemFragment_Consumable.generated.h"

UCLASS()
class SANDBOXINVENTORY_API USBItemFragment_Consumable : public USBItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	FGameplayTag AttributeToModify;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
	float ModifierValue = 0.0f;
};
