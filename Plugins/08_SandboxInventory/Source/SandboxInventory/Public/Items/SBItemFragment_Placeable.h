// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Items/SBItemFragment.h"
#include "SBItemFragment_Placeable.generated.h"

class ASBBuildingPiece;

/**
 * Fragmento de item que permite que ele seja posicionado no mundo como uma estrutura de construção.
 */
UCLASS(BlueprintType)
class SANDBOXINVENTORY_API USBItemFragment_Placeable : public USBItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement")
	TSubclassOf<ASBBuildingPiece> BuildingPieceClass = nullptr;
};
