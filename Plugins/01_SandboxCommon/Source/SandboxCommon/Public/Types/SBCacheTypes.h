// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBCacheTypes.generated.h"

/**
 * Registro compacto de 24 bytes para processamento massivo em lote com cache locality máxima
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCompactEntityRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	int32 EntityID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	int32 TypeID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	float PositionX = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	float PositionY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	float PositionZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	float CustomData = 0.0f;
};

static_assert(sizeof(FSBCompactEntityRecord) == 24, "FSBCompactEntityRecord must be exactly 24 bytes for optimal cache packing");

/**
 * Métricas de diagnóstico de alocação de memória e fragmentação
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMemoryMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	int32 TotalCapacity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	int32 ActiveRecords = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	int32 BytesPerRecord = 24;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	int32 TotalAllocatedBytes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	float FragmentationRatio = 0.0f;
};
