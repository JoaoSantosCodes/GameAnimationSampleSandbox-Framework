// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "SBAsyncSerializationTypes.generated.h"

/**
 * Registro de entidade serializada em binário
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAsyncSaveRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FGuid EntityGuid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString RecordTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<uint8> BinaryData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int64 TimestampTicks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString ChecksumHash;

	FSBAsyncSaveRecord() = default;

	FSBAsyncSaveRecord(const FGuid& InGuid, const FString& InTag, const TArray<uint8>& InData, int64 InTicks = 0, const FString& InHash = TEXT(""))
		: EntityGuid(InGuid)
		, RecordTag(InTag)
		, BinaryData(InData)
		, TimestampTicks(InTicks)
		, ChecksumHash(InHash)
	{
	}
};

/**
 * Bloco consolidado de múltiplos registros serializados
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAsyncSaveChunk
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 ChunkIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FSBAsyncSaveRecord> Records;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 TotalByteSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	bool bIsCompressed = false;
};

/**
 * Métricas e telemetria de persistência e serialização assíncrona
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAsyncSaveMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 TotalSavesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 TotalLoadsCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 TotalBytesSerialized = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 TotalChunksProcessed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float LastAsyncDurationMs = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnAsyncSaveCompleted, bool, bSuccess, int32, BytesWritten);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnAsyncLoadCompleted, bool, bSuccess, int32, RecordsLoaded);
