#include "Subsystems/SBCacheOptimizedBufferSubsystem.h"

void USBCacheOptimizedBufferSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ContiguousBuffer.Empty();
	PreallocatedCapacity = 0;
	PreallocateBuffer(1024);
}

void USBCacheOptimizedBufferSubsystem::Deinitialize()
{
	ContiguousBuffer.Empty();
	Super::Deinitialize();
}

void USBCacheOptimizedBufferSubsystem::PreallocateBuffer(int32 MaxCapacity)
{
	PreallocatedCapacity = FMath::Max(MaxCapacity, 16);
	ContiguousBuffer.Reserve(PreallocatedCapacity);
}

int32 USBCacheOptimizedBufferSubsystem::InsertRecord(const FSBCompactEntityRecord& Record)
{
	return ContiguousBuffer.Add(Record);
}

bool USBCacheOptimizedBufferSubsystem::UpdateRecord(int32 RecordIndex, const FSBCompactEntityRecord& UpdatedRecord)
{
	if (ContiguousBuffer.IsValidIndex(RecordIndex))
	{
		ContiguousBuffer[RecordIndex] = UpdatedRecord;
		return true;
	}
	return false;
}

bool USBCacheOptimizedBufferSubsystem::RemoveRecord(int32 RecordIndex, int32& OutSwappedEntityID)
{
	OutSwappedEntityID = -1;
	if (!ContiguousBuffer.IsValidIndex(RecordIndex))
	{
		return false;
	}

	int32 LastIndex = ContiguousBuffer.Num() - 1;
	if (RecordIndex != LastIndex)
	{
		// Swap with last element and pop to keep buffer strictly dense and contiguous (O(1))
		ContiguousBuffer[RecordIndex] = ContiguousBuffer[LastIndex];
		OutSwappedEntityID = ContiguousBuffer[RecordIndex].EntityID;
	}

	ContiguousBuffer.Pop(EAllowShrinking::No);
	return true;
}

void USBCacheOptimizedBufferSubsystem::ProcessRecordsZeroAlloc(TFunctionRef<void(const FSBCompactEntityRecord&)> Predicate) const
{
	const FSBCompactEntityRecord* BufferPtr = ContiguousBuffer.GetData();
	const int32 Total = ContiguousBuffer.Num();
	for (int32 i = 0; i < Total; ++i)
	{
		Predicate(BufferPtr[i]);
	}
}

FSBMemoryMetrics USBCacheOptimizedBufferSubsystem::GetMemoryMetrics() const
{
	FSBMemoryMetrics Metrics;
	Metrics.TotalCapacity = ContiguousBuffer.Max();
	Metrics.ActiveRecords = ContiguousBuffer.Num();
	Metrics.BytesPerRecord = sizeof(FSBCompactEntityRecord);
	Metrics.TotalAllocatedBytes = Metrics.TotalCapacity * Metrics.BytesPerRecord;
	Metrics.FragmentationRatio = 0.0f; // Always 0 in contiguous flat buffers
	return Metrics;
}

int32 USBCacheOptimizedBufferSubsystem::GetRecordCount() const
{
	return ContiguousBuffer.Num();
}

FSBCompactEntityRecord USBCacheOptimizedBufferSubsystem::GetRecord(int32 Index) const
{
	if (ContiguousBuffer.IsValidIndex(Index))
	{
		return ContiguousBuffer[Index];
	}
	return FSBCompactEntityRecord();
}

int32 USBCacheOptimizedBufferSubsystem::FindRecordIndexByEntityID(int32 EntityID) const
{
	for (int32 i = 0; i < ContiguousBuffer.Num(); ++i)
	{
		if (ContiguousBuffer[i].EntityID == EntityID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}
