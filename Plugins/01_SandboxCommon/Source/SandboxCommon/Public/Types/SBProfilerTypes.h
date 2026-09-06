#pragma once

#include "CoreMinimal.h"
#include "SBProfilerTypes.generated.h"

/**
 * Registro de telemetria estatística de desempenho de um componente
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBComponentSampleData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	FName ComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	float LastExecutionDurationUs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	float AverageDurationUs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	float MaxDurationUs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	float MinDurationUs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	int32 SampleCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	int64 EstimatedMemoryBytes = 0;

	FSBComponentSampleData() = default;

	FSBComponentSampleData(FName InName)
		: ComponentName(InName) {}
};

/**
 * Métricas consolidadas do profiler de desempenho
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBPerformanceProfilerMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	int32 TotalComponentsTracked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	int32 TotalProfilingSamplesRecorded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	float TotalFrameBudgetSpentUs = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerformanceProfiler")
	int32 HotComponentsCount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBOnComponentBudgetExceeded, FName, ComponentName, float, ActualDurationUs, float, BudgetThresholdUs);
