// Copyright 2026 João Santos. All Rights Reserved.
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/SBItemFragment.h"
#include "SBItemFragment_Salvageable.generated.h"

class USBItemDefinition;

/**
 * Resultado potencial ao desmantelar o item.
 */
USTRUCT(BlueprintType)
struct FSBSalvageOutcome
{
	GENERATED_BODY()

	// Tipo de item a ser concedido
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salvage")
	TObjectPtr<USBItemDefinition> ItemDef = nullptr;

	// Quantidade mínima de itens gerada por ciclo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salvage", meta = (ClampMin = "1"))
	int32 MinQuantity = 1;

	// Quantidade máxima de itens gerada por ciclo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salvage", meta = (ClampMin = "1"))
	int32 MaxQuantity = 1;

	// Probabilidade do drop ocorrer (0.0f a 1.0f)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salvage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Probability = 1.0f;
};

/**
 * Fragmento de item que o torna desmantelável (salvageable).
 */
UCLASS()
class SANDBOXINVENTORY_API USBItemFragment_Salvageable : public USBItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Salvage")
	TArray<FSBSalvageOutcome> PotentialOutcomes;
};
