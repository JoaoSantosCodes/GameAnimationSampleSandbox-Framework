#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBThreadingTypes.h"
#include "SBAsyncTaskManagerSubsystem.generated.h"

/**
 * Subsistema de despacho de tarefas assíncronas do Task Graph e processamento em lote com ParallelFor
 */
UCLASS()
class SANDBOXCORE_API USBAsyncTaskManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Parallel Batch Processing using ParallelFor
	void DispatchParallelTransformBatch(TArray<FVector>& InOutLocations, const FVector& TranslationDelta);

	// Async Background Execution using Async/Task Graph
	void DispatchAsyncBatchCalculation(
		const TArray<float>& InputData,
		TFunction<float(float)> ComputeFunction,
		TFunction<void(const TArray<float>&)> OnCompleteCallback
	);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Threading")
	FSBMultiThreadMetrics GetMetrics() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Threading")
	int32 GetActiveTaskCount() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Threading")
	int32 GetTotalCompletedTasks() const;

private:
	mutable FCriticalSection MetricsLock;

	UPROPERTY()
	FSBMultiThreadMetrics Metrics;
};
