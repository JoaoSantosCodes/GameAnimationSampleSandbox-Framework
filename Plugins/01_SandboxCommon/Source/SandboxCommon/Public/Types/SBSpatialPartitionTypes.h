// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBSpatialPartitionTypes.generated.h"

/**
 * Coordenada tridimensional inteira de uma célula espacial
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSpatialCellCoord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	int32 Y = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	int32 Z = 0;

	FSBSpatialCellCoord() : X(0), Y(0), Z(0) {}
	FSBSpatialCellCoord(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}

	bool operator==(const FSBSpatialCellCoord& Other) const
	{
		return X == Other.X && Y == Other.Y && Z == Other.Z;
	}

	bool operator!=(const FSBSpatialCellCoord& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FSBSpatialCellCoord& Coord)
	{
		return HashCombine(HashCombine(GetTypeHash(Coord.X), GetTypeHash(Coord.Y)), GetTypeHash(Coord.Z));
	}
};

/**
 * Elemento espacial indexado em uma célula da grid
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSpatialEntityElement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	int32 EntityID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	int32 EntityType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	float BoundingRadius = 50.0f;
};

/**
 * Métricas de diagnóstico de particionamento espacial
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSpatialGridMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	int32 TotalCellsActive = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	int32 TotalIndexedEntities = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	float CellSize = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial")
	int32 MaxEntitiesInSingleCell = 0;
};
