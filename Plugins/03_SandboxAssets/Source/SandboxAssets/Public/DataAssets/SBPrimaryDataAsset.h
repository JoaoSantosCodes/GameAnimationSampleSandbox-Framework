// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SBPrimaryDataAsset.generated.h"

UCLASS(Abstract, BlueprintType)
class SANDBOXASSETS_API USBPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
