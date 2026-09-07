// Copyright 2026 João Santos. All Rights Reserved.
#include "DataAssets/SBLootTableDataAsset.h"

USBLootTableDataAsset::USBLootTableDataAsset()
{
}

TArray<FSBLootDropResult> USBLootTableDataAsset::RollLoot(int32 NumberOfRolls) const
{
	TArray<FSBLootDropResult> Results;
	if (Entries.Num() == 0 || NumberOfRolls <= 0)
	{
		return Results;
	}

	// 1. Filtra entradas válidas e calcula o peso total acumulado
	float TotalWeight = 0.0f;
	TArray<const FSBLootEntry*> ValidEntries;

	for (const FSBLootEntry& Entry : Entries)
	{
		if (Entry.ItemDefinition && Entry.Weight > 0.0f)
		{
			ValidEntries.Add(&Entry);
			TotalWeight += Entry.Weight;
		}
	}

	if (ValidEntries.Num() == 0 || TotalWeight <= 0.0f)
	{
		return Results;
	}

	// 2. Executa cada roll
	for (int32 i = 0; i < NumberOfRolls; ++i)
	{
		float RandomWeight = FMath::FRandRange(0.0f, TotalWeight);
		float AccumulatedWeight = 0.0f;

		for (const FSBLootEntry* Entry : ValidEntries)
		{
			AccumulatedWeight += Entry->Weight;
			if (RandomWeight <= AccumulatedWeight)
			{
				// Verifica a chance de drop individual (0.0 a 1.0)
				if (Entry->DropChance >= 1.0f || FMath::FRand() <= Entry->DropChance)
				{
					int32 Stack = FMath::RandRange(Entry->MinStackCount, Entry->MaxStackCount);
					if (Stack > 0)
					{
						FSBLootDropResult DropResult;
						DropResult.ItemDefinition = Entry->ItemDefinition;
						DropResult.StackCount = Stack;
						Results.Add(DropResult);
					}
				}
				break;
			}
		}
	}

	return Results;
}
