// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/SBItemDefinition.h"
#include "SBLootTableDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FSBLootEntry
{
	GENERATED_BODY()

	// Definição do item a ser gerado
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<USBItemDefinition> ItemDefinition = nullptr;

	// Quantidade mínima no drop
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 MinStackCount = 1;

	// Quantidade máxima no drop
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 MaxStackCount = 1;

	// Peso relativo para sorteio ponderado
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0.01"))
	float Weight = 1.0f;

	// Chance independente de drop (0.0 = nunca, 1.0 = 100%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 1.0f;
};

USTRUCT(BlueprintType)
struct FSBLootDropResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	TObjectPtr<USBItemDefinition> ItemDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Loot")
	int32 StackCount = 0;
};

UCLASS(BlueprintType)
class SANDBOXINVENTORY_API USBLootTableDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USBLootTableDataAsset();

	// Lista de possíveis entradas de drop nesta tabela
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	TArray<FSBLootEntry> Entries;

	// Executa os sorteios ponderados e retorna a lista de itens gerados
	UFUNCTION(BlueprintCallable, Category = "Loot")
	TArray<FSBLootDropResult> RollLoot(int32 NumberOfRolls = 1) const;
};
