#pragma once

#include "CoreMinimal.h"
#include "Items/SBItemFragment.h"
#include "GameplayTagContainer.h"
#include "DataAssets/SBCraftingRecipeDataAsset.h" // For FSBCraftingIngredient
#include "SBItemFragment_Upgrade.generated.h"

USTRUCT(BlueprintType)
struct FSBUpgradeCostPerLevel
{
	GENERATED_BODY()

	// Lista de ingredientes necessários para este nível de upgrade
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	TArray<FSBCraftingIngredient> Ingredients;

	// Custo em moedas (Coins) para este nível de upgrade
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	int32 CoinCost = 0;

	// Multiplicador de magnitude extra para os modificadores deste nível (ex: 0.1f para +10% adicionais)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	float StatMultiplierBonus = 0.1f;
};

UCLASS(EditInlineNew, DefaultToInstanced)
class SANDBOXINVENTORY_API USBItemFragment_Upgrade : public USBItemFragment
{
	GENERATED_BODY()

public:
	// Lista ordenada de custos por nível de upgrade (Índice 0 = upgrade +1, Índice 1 = upgrade +2, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	TArray<FSBUpgradeCostPerLevel> UpgradeLevels;

	// Tag da estação necessária para efetuar o upgrade (ex: Crafting.Station.Forge)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	FGameplayTag RequiredStationTag;
};
