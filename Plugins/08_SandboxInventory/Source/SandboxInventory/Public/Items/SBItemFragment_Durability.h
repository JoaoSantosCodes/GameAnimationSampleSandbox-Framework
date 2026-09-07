// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Items/SBItemFragment.h"
#include "SBItemFragment_Durability.generated.h"

UCLASS(BlueprintType, meta = (DisplayName = "Durability Fragment"))
class SANDBOXINVENTORY_API USBItemFragment_Durability : public USBItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Durability", meta = (ClampMin = "0.0"))
	float MaxDurability = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Durability", meta = (ClampMin = "0.0"))
	float InitialDurability = 100.0f;
};
