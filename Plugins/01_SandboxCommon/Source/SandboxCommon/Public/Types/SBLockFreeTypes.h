// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBLockFreeTypes.generated.h"

/**
 * Evento de payload fixo para enfileiramento lock-free e barramento atômico
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLockFreeEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	int32 EventID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	int32 SourceEntityID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	int32 EventType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	float PayloadFloat = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	int64 TimestampTicks = 0;

	FSBLockFreeEvent() = default;

	FSBLockFreeEvent(int32 InEventID, int32 InSourceEntityID, int32 InEventType, float InPayloadFloat, int64 InTimestampTicks = 0)
		: EventID(InEventID)
		, SourceEntityID(InSourceEntityID)
		, EventType(InEventType)
		, PayloadFloat(InPayloadFloat)
		, TimestampTicks(InTimestampTicks)
	{
	}
};

/**
 * Métricas e telemetria de alta performance para a fila lock-free
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLockFreeQueueMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	int32 QueueCapacity = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	int32 EnqueuedEventsCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	int32 DequeuedEventsCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	int32 DroppedEventsCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	int32 PendingCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockFree")
	bool bIsOverflown = false;
};
