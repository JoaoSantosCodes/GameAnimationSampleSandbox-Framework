// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Items/SBItemDefinition.h"
#include "SBCraftingRecipeDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FSBCraftingIngredient
{
	GENERATED_BODY()

	// Definição do item insumo necessário para a receita
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
	TObjectPtr<USBItemDefinition> ItemDef = nullptr;

	// Quantidade exigida deste item para a confecção
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting", meta = (ClampMin = "1"))
	int32 Quantity = 1;
};

UCLASS(BlueprintType)
class SANDBOXINVENTORY_API USBCraftingRecipeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USBCraftingRecipeDataAsset();

	// Identificador único da receita via GameplayTag (ex: Crafting.Recipe.IronSword)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	FGameplayTag RecipeTag;

	// Nome amigável de exibição da receita
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	FText DisplayName;

	// Estação de trabalho obrigatória para fabricar este item (ex: Crafting.Station.Forge). Vazio se dispensar estação.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	FGameplayTag RequiredStationTag;

	// Lista de ingredientes necessários para a fabricação
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	TArray<FSBCraftingIngredient> Ingredients;

	// Definição do item resultante a ser criado
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	TObjectPtr<USBItemDefinition> ResultItemDef = nullptr;

	// Quantidade de itens gerados por ciclo de criação
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting", meta = (ClampMin = "1"))
	int32 ResultQuantity = 1;

	// Duração em segundos do processo de criação (0.0f = instantâneo)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting", meta = (ClampMin = "0.0"))
	float CraftingDuration = 0.0f;
};
