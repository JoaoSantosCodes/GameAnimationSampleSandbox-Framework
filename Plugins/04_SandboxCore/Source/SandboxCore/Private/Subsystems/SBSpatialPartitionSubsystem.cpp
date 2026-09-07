// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBSpatialPartitionSubsystem.h"

void USBSpatialPartitionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CellSize = 1000.0f;
	GridBuckets.Empty();
}

void USBSpatialPartitionSubsystem::Deinitialize()
{
	GridBuckets.Empty();
	Super::Deinitialize();
}

void USBSpatialPartitionSubsystem::SetCellSize(float NewCellSize)
{
	if (NewCellSize > 1.0f)
	{
		CellSize = NewCellSize;
	}
}

FSBSpatialCellCoord USBSpatialPartitionSubsystem::WorldToCell(const FVector& WorldPos) const
{
	const float SafeCellSize = FMath::Max(1.0f, CellSize);
	const int32 CellX = FMath::FloorToInt(WorldPos.X / SafeCellSize);
	const int32 CellY = FMath::FloorToInt(WorldPos.Y / SafeCellSize);
	const int32 CellZ = FMath::FloorToInt(WorldPos.Z / SafeCellSize);
	return FSBSpatialCellCoord(CellX, CellY, CellZ);
}

void USBSpatialPartitionSubsystem::RegisterEntity(const FSBSpatialEntityElement& Entity)
{
	const FSBSpatialCellCoord Cell = WorldToCell(Entity.Location);
	TArray<FSBSpatialEntityElement>& Bucket = GridBuckets.FindOrAdd(Cell);

	Bucket.RemoveAll([Entity](const FSBSpatialEntityElement& E)
	{
		return E.EntityID == Entity.EntityID;
	});

	Bucket.Add(Entity);
}

void USBSpatialPartitionSubsystem::UnregisterEntity(int32 EntityID, const FVector& Location)
{
	const FSBSpatialCellCoord Cell = WorldToCell(Location);
	if (TArray<FSBSpatialEntityElement>* Bucket = GridBuckets.Find(Cell))
	{
		Bucket->RemoveAll([EntityID](const FSBSpatialEntityElement& E)
		{
			return E.EntityID == EntityID;
		});

		if (Bucket->Num() == 0)
		{
			GridBuckets.Remove(Cell);
		}
	}
}

void USBSpatialPartitionSubsystem::UpdateEntityLocation(int32 EntityID, const FVector& OldLocation, const FVector& NewLocation)
{
	const FSBSpatialCellCoord OldCell = WorldToCell(OldLocation);
	const FSBSpatialCellCoord NewCell = WorldToCell(NewLocation);

	if (OldCell == NewCell)
	{
		if (TArray<FSBSpatialEntityElement>* Bucket = GridBuckets.Find(OldCell))
		{
			for (FSBSpatialEntityElement& E : *Bucket)
			{
				if (E.EntityID == EntityID)
				{
					E.Location = NewLocation;
					break;
				}
			}
		}
	}
	else
	{
		FSBSpatialEntityElement ElementCopy;
		bool bFound = false;

		if (TArray<FSBSpatialEntityElement>* OldBucket = GridBuckets.Find(OldCell))
		{
			for (int32 i = 0; i < OldBucket->Num(); ++i)
			{
				if ((*OldBucket)[i].EntityID == EntityID)
				{
					ElementCopy = (*OldBucket)[i];
					ElementCopy.Location = NewLocation;
					bFound = true;
					OldBucket->RemoveAtSwap(i);
					break;
				}
			}

			if (OldBucket->Num() == 0)
			{
				GridBuckets.Remove(OldCell);
			}
		}

		if (bFound)
		{
			TArray<FSBSpatialEntityElement>& NewBucket = GridBuckets.FindOrAdd(NewCell);
			NewBucket.Add(ElementCopy);
		}
	}
}

TArray<FSBSpatialEntityElement> USBSpatialPartitionSubsystem::FindEntitiesInRadius(const FVector& Origin, float Radius, int32 FilterType) const
{
	TArray<FSBSpatialEntityElement> Results;
	const float SafeCellSize = FMath::Max(1.0f, CellSize);
	const float RadiusSq = Radius * Radius;

	const int32 MinX = FMath::FloorToInt((Origin.X - Radius) / SafeCellSize);
	const int32 MaxX = FMath::FloorToInt((Origin.X + Radius) / SafeCellSize);
	const int32 MinY = FMath::FloorToInt((Origin.Y - Radius) / SafeCellSize);
	const int32 MaxY = FMath::FloorToInt((Origin.Y + Radius) / SafeCellSize);
	const int32 MinZ = FMath::FloorToInt((Origin.Z - Radius) / SafeCellSize);
	const int32 MaxZ = FMath::FloorToInt((Origin.Z + Radius) / SafeCellSize);

	for (int32 X = MinX; X <= MaxX; ++X)
	{
		for (int32 Y = MinY; Y <= MaxY; ++Y)
		{
			for (int32 Z = MinZ; Z <= MaxZ; ++Z)
			{
				const FSBSpatialCellCoord Cell(X, Y, Z);
				if (const TArray<FSBSpatialEntityElement>* Bucket = GridBuckets.Find(Cell))
				{
					for (const FSBSpatialEntityElement& Entity : *Bucket)
					{
						if (FilterType != 0 && Entity.EntityType != FilterType)
						{
							continue;
						}

						if (FVector::DistSquared(Origin, Entity.Location) <= RadiusSq)
						{
							Results.Add(Entity);
						}
					}
				}
			}
		}
	}

	return Results;
}

TArray<FSBSpatialEntityElement> USBSpatialPartitionSubsystem::FindEntitiesInBox(const FBox& BoundingBox, int32 FilterType) const
{
	TArray<FSBSpatialEntityElement> Results;
	const float SafeCellSize = FMath::Max(1.0f, CellSize);

	const int32 MinX = FMath::FloorToInt(BoundingBox.Min.X / SafeCellSize);
	const int32 MaxX = FMath::FloorToInt(BoundingBox.Max.X / SafeCellSize);
	const int32 MinY = FMath::FloorToInt(BoundingBox.Min.Y / SafeCellSize);
	const int32 MaxY = FMath::FloorToInt(BoundingBox.Max.Y / SafeCellSize);
	const int32 MinZ = FMath::FloorToInt(BoundingBox.Min.Z / SafeCellSize);
	const int32 MaxZ = FMath::FloorToInt(BoundingBox.Max.Z / SafeCellSize);

	for (int32 X = MinX; X <= MaxX; ++X)
	{
		for (int32 Y = MinY; Y <= MaxY; ++Y)
		{
			for (int32 Z = MinZ; Z <= MaxZ; ++Z)
			{
				const FSBSpatialCellCoord Cell(X, Y, Z);
				if (const TArray<FSBSpatialEntityElement>* Bucket = GridBuckets.Find(Cell))
				{
					for (const FSBSpatialEntityElement& Entity : *Bucket)
					{
						if (FilterType != 0 && Entity.EntityType != FilterType)
						{
							continue;
						}

						if (BoundingBox.IsInside(Entity.Location))
						{
							Results.Add(Entity);
						}
					}
				}
			}
		}
	}

	return Results;
}

FSBSpatialGridMetrics USBSpatialPartitionSubsystem::GetMetrics() const
{
	FSBSpatialGridMetrics Metrics;
	Metrics.CellSize = CellSize;
	Metrics.TotalCellsActive = GridBuckets.Num();
	Metrics.TotalIndexedEntities = 0;
	Metrics.MaxEntitiesInSingleCell = 0;

	for (const auto& Pair : GridBuckets)
	{
		const int32 Count = Pair.Value.Num();
		Metrics.TotalIndexedEntities += Count;
		Metrics.MaxEntitiesInSingleCell = FMath::Max(Metrics.MaxEntitiesInSingleCell, Count);
	}

	return Metrics;
}

void USBSpatialPartitionSubsystem::ClearAllEntities()
{
	GridBuckets.Empty();
}
