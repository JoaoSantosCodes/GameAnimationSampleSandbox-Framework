// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBSpatialPartitionTypes.h"
#include "SBSpatialPartitionSubsystem.generated.h"

/**
 * Subsistema de mundo para particionamento espacial em grade tridimensional hashada
 */
UCLASS()
class SANDBOXCORE_API USBSpatialPartitionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Grid configuration
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Spatial")
	void SetCellSize(float NewCellSize);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Spatial")
	float GetCellSize() const { return CellSize; }

	// Spatial coordinate conversion
	FSBSpatialCellCoord WorldToCell(const FVector& WorldPos) const;

	// Entity Registration & Management
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Spatial")
	void RegisterEntity(const FSBSpatialEntityElement& Entity);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Spatial")
	void UnregisterEntity(int32 EntityID, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Spatial")
	void UpdateEntityLocation(int32 EntityID, const FVector& OldLocation, const FVector& NewLocation);

	// Spatial Queries
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Spatial")
	TArray<FSBSpatialEntityElement> FindEntitiesInRadius(const FVector& Origin, float Radius, int32 FilterType = 0) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Spatial")
	TArray<FSBSpatialEntityElement> FindEntitiesInBox(const FBox& BoundingBox, int32 FilterType = 0) const;

	// Metrics
	UFUNCTION(BlueprintPure, Category = "Sandbox|Spatial")
	FSBSpatialGridMetrics GetMetrics() const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Spatial")
	void ClearAllEntities();

private:
	float CellSize;
	TMap<FSBSpatialCellCoord, TArray<FSBSpatialEntityElement>> GridBuckets;
};
