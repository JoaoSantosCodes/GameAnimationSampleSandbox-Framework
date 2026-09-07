// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBItemDefinition.generated.h"

class USBItemFragment;

UCLASS(BlueprintType, Const)
class SANDBOXINVENTORY_API USBItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stacking")
	int32 MaxStackCount = 1;

	/**
	 * Icone do item na interface. Soft pointer: a textura so carrega quando a UI pede.
	 *
	 * Vazio e um estado valido — o slot desenha um quadrado com a cor da raridade, de modo que
	 * um item sem arte ainda aparece no inventario em vez de virar um buraco na grade.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Fragments")
	TArray<TObjectPtr<USBItemFragment>> Fragments;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const USBItemFragment* FindFragmentByClass(TSubclassOf<USBItemFragment> FragmentClass) const;
};
