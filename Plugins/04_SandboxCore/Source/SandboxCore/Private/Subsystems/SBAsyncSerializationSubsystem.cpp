#include "Subsystems/SBAsyncSerializationSubsystem.h"
#include "Misc/SecureHash.h"
#include "Misc/DateTime.h"

USBAsyncSerializationSubsystem::USBAsyncSerializationSubsystem()
{
}

void USBAsyncSerializationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetSubsystem();
}

void USBAsyncSerializationSubsystem::Deinitialize()
{
	ResetSubsystem();
	Super::Deinitialize();
}

FString USBAsyncSerializationSubsystem::ComputePayloadHash(const TArray<uint8>& BinaryData)
{
	if (BinaryData.Num() == 0)
	{
		return TEXT("empty");
	}

	FMD5 MD5Gen;
	MD5Gen.Update(BinaryData.GetData(), BinaryData.Num());
	uint8 Digest[16];
	MD5Gen.Final(Digest);
	return BytesToHex(Digest, 16);
}

int32 USBAsyncSerializationSubsystem::SerializeSnapshotSync(const TArray<FSBAsyncSaveRecord>& InRecords, FName SlotName)
{
	FSBAsyncSaveChunk Chunk;
	Chunk.ChunkIndex = 0;
	int32 ByteCount = 0;

	for (const FSBAsyncSaveRecord& InRecord : InRecords)
	{
		FSBAsyncSaveRecord Processed = InRecord;
		if (Processed.ChecksumHash.IsEmpty())
		{
			Processed.ChecksumHash = ComputePayloadHash(Processed.BinaryData);
		}
		if (Processed.TimestampTicks == 0)
		{
			Processed.TimestampTicks = FDateTime::UtcNow().GetTicks();
		}

		ByteCount += Processed.BinaryData.Num();
		Chunk.Records.Add(Processed);
	}

	Chunk.TotalByteSize = ByteCount;
	SavedSlots.Add(SlotName, Chunk);

	TotalSavesCompleted++;
	TotalBytesSerialized += ByteCount;
	TotalChunksProcessed++;

	OnAsyncSaveCompleted.Broadcast(true, ByteCount);
	return ByteCount;
}

bool USBAsyncSerializationSubsystem::DeserializeSnapshotSync(FName SlotName, TArray<FSBAsyncSaveRecord>& OutRecords)
{
	FSBAsyncSaveChunk* Chunk = SavedSlots.Find(SlotName);
	if (!Chunk)
	{
		OnAsyncLoadCompleted.Broadcast(false, 0);
		return false;
	}

	OutRecords.Reset();
	for (const FSBAsyncSaveRecord& Rec : Chunk->Records)
	{
		FString CurrentHash = ComputePayloadHash(Rec.BinaryData);
		if (!Rec.ChecksumHash.IsEmpty() && CurrentHash != Rec.ChecksumHash)
		{
			// Detectou corrupção de dados
			OnAsyncLoadCompleted.Broadcast(false, 0);
			OutRecords.Reset();
			return false;
		}
		OutRecords.Add(Rec);
	}

	TotalLoadsCompleted++;
	OnAsyncLoadCompleted.Broadcast(true, OutRecords.Num());
	return true;
}

FSBAsyncSaveMetrics USBAsyncSerializationSubsystem::GetMetrics() const
{
	FSBAsyncSaveMetrics Metrics;
	Metrics.TotalSavesCompleted = TotalSavesCompleted;
	Metrics.TotalLoadsCompleted = TotalLoadsCompleted;
	Metrics.TotalBytesSerialized = TotalBytesSerialized;
	Metrics.TotalChunksProcessed = TotalChunksProcessed;
	Metrics.LastAsyncDurationMs = 0.0f;
	return Metrics;
}

bool USBAsyncSerializationSubsystem::HasSlot(FName SlotName) const
{
	return SavedSlots.Contains(SlotName);
}

void USBAsyncSerializationSubsystem::ClearSlot(FName SlotName)
{
	SavedSlots.Remove(SlotName);
}

void USBAsyncSerializationSubsystem::ResetSubsystem()
{
	SavedSlots.Empty();
	TotalSavesCompleted = 0;
	TotalLoadsCompleted = 0;
	TotalBytesSerialized = 0;
	TotalChunksProcessed = 0;
}
