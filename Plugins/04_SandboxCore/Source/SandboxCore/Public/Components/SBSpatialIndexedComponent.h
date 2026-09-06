#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBSpatialPartitionTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBSpatialIndexedComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FSBSpatialCellChanged, int32, OldCellX, int32, OldCellY, int32, OldCellZ, int32, NewCellX, int32, NewCellY, int32, NewCellZ);

/**
 * Componente para registro automático e rastreamento de entidade na grade de particionamento espacial
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCORE_API USBSpatialIndexedComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBSpatialIndexedComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Controls
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Spatial")
	void SetupSpatialEntity(int32 InEntityID, int32 InEntityType, float InRadius);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Spatial")
	void SyncLocationToSpatialGrid();

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Spatial")
	FSBSpatialCellCoord GetCurrentCellCoord() const { return CurrentCell; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Spatial")
	int32 GetEntityID() const { return EntityID; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Spatial")
	int32 GetEntityType() const { return EntityType; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Spatial")
	FSBSpatialCellChanged OnSpatialCellChanged;

private:
	void SyncTags();

	UPROPERTY()
	int32 EntityID;

	UPROPERTY()
	int32 EntityType;

	UPROPERTY()
	float BoundingRadius;

	UPROPERTY()
	FVector LastRegisteredLocation;

	UPROPERTY()
	FSBSpatialCellCoord CurrentCell;

	UPROPERTY()
	bool bIsRegistered;

	UPROPERTY()
	TWeakObjectPtr<UActorComponent> CachedStateComp;
};
