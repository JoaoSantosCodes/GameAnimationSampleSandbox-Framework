// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBThreadingTypes.generated.h"

/**
 * Nível de prioridade de tarefas assíncronas
 */
UENUM(BlueprintType)
enum class ESBAsyncPriority : uint8
{
	High UMETA(DisplayName = "High Priority"),
	Normal UMETA(DisplayName = "Normal Priority"),
	Background UMETA(DisplayName = "Background Priority")
};

/**
 * Payload de trabalho assíncrono para Task Graph e Worker Pools
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAsyncWorkPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	int32 WorkID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	int32 BatchSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	float TotalProcessingTimeMs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	bool bIsCompleted = false;
};

/**
 * Métricas de monitoramento de concorrência e processamento paralelo
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMultiThreadMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	int32 ActiveAsyncTasks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	int32 TotalTasksCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	float AverageTaskExecutionTimeMs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
	int32 ParallelBatchSize = 64;
};
