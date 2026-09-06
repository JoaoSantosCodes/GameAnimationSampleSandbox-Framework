#pragma once

#include "CoreMinimal.h"
#include "Items/SBItemFragment.h"
#include "GameplayTagContainer.h"
#include "SBItemFragment_Rarity.generated.h"

UCLASS(EditInlineNew, DefaultToInstanced)
class SANDBOXINVENTORY_API USBItemFragment_Rarity : public USBItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	FGameplayTag RarityTag;
};
