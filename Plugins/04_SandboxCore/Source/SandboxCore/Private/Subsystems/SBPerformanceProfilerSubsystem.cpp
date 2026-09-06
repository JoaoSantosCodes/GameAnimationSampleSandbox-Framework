#include "Subsystems/SBPerformanceProfilerSubsystem.h"

USBPerformanceProfilerSubsystem::USBPerformanceProfilerSubsystem()
{
}

void USBPerformanceProfilerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetSubsystem();
}

void USBPerformanceProfilerSubsystem::Deinitialize()
{
	ResetSubsystem();
	Super::Deinitialize();
}

void USBPerformanceProfilerSubsystem::RecordComponentSample(FName ComponentName, float DurationUs, int64 MemoryBytes)
{
	FSBComponentSampleData& Stats = ComponentStatsMap.FindOrAdd(ComponentName);
	if (Stats.SampleCount == 0)
	{
		Stats.ComponentName = ComponentName;
		Stats.MinDurationUs = DurationUs;
		Stats.MaxDurationUs = DurationUs;
		Stats.AverageDurationUs = DurationUs;
	}
	else
	{
		Stats.MinDurationUs = FMath::Min(Stats.MinDurationUs, DurationUs);
		Stats.MaxDurationUs = FMath::Max(Stats.MaxDurationUs, DurationUs);
		Stats.AverageDurationUs = (Stats.AverageDurationUs * Stats.SampleCount + DurationUs) / (Stats.SampleCount + 1);
	}
	Stats.LastExecutionDurationUs = DurationUs;
	Stats.EstimatedMemoryBytes = MemoryBytes;
	Stats.SampleCount++;

	TotalSamplesRecorded++;
	TotalTimeBudgetSpent += DurationUs;

	if (const float* Threshold = BudgetThresholdsMap.Find(ComponentName))
	{
		if (DurationUs > *Threshold)
		{
			OnComponentBudgetExceeded.Broadcast(ComponentName, DurationUs, *Threshold);
		}
	}
}

void USBPerformanceProfilerSubsystem::SetBudgetThreshold(FName ComponentName, float BudgetThresholdUs)
{
	BudgetThresholdsMap.Add(ComponentName, BudgetThresholdUs);
}

FSBComponentSampleData USBPerformanceProfilerSubsystem::GetComponentStats(FName ComponentName) const
{
	if (const FSBComponentSampleData* Found = ComponentStatsMap.Find(ComponentName))
	{
		return *Found;
	}
	return FSBComponentSampleData(ComponentName);
}

TArray<FSBComponentSampleData> USBPerformanceProfilerSubsystem::GetHotComponents(float ThresholdUs) const
{
	TArray<FSBComponentSampleData> Result;
	for (const auto& Pair : ComponentStatsMap)
	{
		if (Pair.Value.MaxDurationUs >= ThresholdUs || Pair.Value.AverageDurationUs >= ThresholdUs)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FSBPerformanceProfilerMetrics USBPerformanceProfilerSubsystem::GetMetrics() const
{
	FSBPerformanceProfilerMetrics Metrics;
	Metrics.TotalComponentsTracked = ComponentStatsMap.Num();
	Metrics.TotalProfilingSamplesRecorded = TotalSamplesRecorded;
	Metrics.TotalFrameBudgetSpentUs = TotalTimeBudgetSpent;

	int32 HotCount = 0;
	for (const auto& Pair : ComponentStatsMap)
	{
		if (const float* Thresh = BudgetThresholdsMap.Find(Pair.Key))
		{
			if (Pair.Value.MaxDurationUs > *Thresh)
			{
				HotCount++;
			}
		}
	}
	Metrics.HotComponentsCount = HotCount;

	return Metrics;
}

void USBPerformanceProfilerSubsystem::ResetSubsystem()
{
	ComponentStatsMap.Empty();
	BudgetThresholdsMap.Empty();
	TotalSamplesRecorded = 0;
	TotalTimeBudgetSpent = 0.0f;
}
