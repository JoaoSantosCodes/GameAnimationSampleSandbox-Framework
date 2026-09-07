// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBAsyncTaskManagerSubsystem.h"
#include "Async/ParallelFor.h"
#include "Async/Async.h"

void USBAsyncTaskManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Metrics.ActiveAsyncTasks = 0;
	Metrics.TotalTasksCompleted = 0;
	Metrics.AverageTaskExecutionTimeMs = 0.0f;
	Metrics.ParallelBatchSize = 64;
}

void USBAsyncTaskManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USBAsyncTaskManagerSubsystem::DispatchParallelTransformBatch(TArray<FVector>& InOutLocations, const FVector& TranslationDelta)
{
	const int32 Total = InOutLocations.Num();
	if (Total == 0)
	{
		return;
	}

	FVector* DataPtr = InOutLocations.GetData();
	ParallelFor(Total, [DataPtr, TranslationDelta](int32 Index)
	{
		DataPtr[Index] += TranslationDelta;
	});

	{
		FScopeLock Lock(&MetricsLock);
		Metrics.TotalTasksCompleted++;
	}
}

void USBAsyncTaskManagerSubsystem::DispatchAsyncBatchCalculation(
	const TArray<float>& InputData,
	TFunction<float(float)> ComputeFunction,
	TFunction<void(const TArray<float>&)> OnCompleteCallback)
{
	{
		FScopeLock Lock(&MetricsLock);
		Metrics.ActiveAsyncTasks++;
	}

	// mutable: o operator() do lambda é const por padrão, o que impediria mover a
	// callback capturada para dentro da task despachada ao Game Thread.
	Async(EAsyncExecution::ThreadPool, [this, InputData, ComputeFunction = MoveTemp(ComputeFunction), OnCompleteCallback = MoveTemp(OnCompleteCallback)]() mutable
	{
		TArray<float> Results;
		Results.SetNumUninitialized(InputData.Num());

		for (int32 i = 0; i < InputData.Num(); ++i)
		{
			Results[i] = ComputeFunction(InputData[i]);
		}

		// Dispatch callback back to Game Thread
		FFunctionGraphTask::CreateAndDispatchWhenReady([this, Results = MoveTemp(Results), OnCompleteCallback = MoveTemp(OnCompleteCallback)]()
		{
			{
				FScopeLock Lock(&MetricsLock);
				Metrics.ActiveAsyncTasks = FMath::Max(0, Metrics.ActiveAsyncTasks - 1);
				Metrics.TotalTasksCompleted++;
			}

			if (OnCompleteCallback)
			{
				OnCompleteCallback(Results);
			}
		}, TStatId(), nullptr, ENamedThreads::GameThread);
	});
}

FSBMultiThreadMetrics USBAsyncTaskManagerSubsystem::GetMetrics() const
{
	FScopeLock Lock(&MetricsLock);
	return Metrics;
}

int32 USBAsyncTaskManagerSubsystem::GetActiveTaskCount() const
{
	FScopeLock Lock(&MetricsLock);
	return Metrics.ActiveAsyncTasks;
}

int32 USBAsyncTaskManagerSubsystem::GetTotalCompletedTasks() const
{
	FScopeLock Lock(&MetricsLock);
	return Metrics.TotalTasksCompleted;
}
