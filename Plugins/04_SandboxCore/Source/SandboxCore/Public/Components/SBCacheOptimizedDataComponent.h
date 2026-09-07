// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBCacheTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBCacheOptimizedDataComponent.generated.h"

class USBCacheOptimizedBufferSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBRecordIndexUpdated, int32, NewIndex);

/**
 * Componente que mapeia o ator para um registro contíguo no buffer de cache otimizado
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCORE_API USBCacheOptimizedDataComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBCacheOptimizedDataComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Controls
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Memory")
	void SetupRecord(int32 InEntityID, int32 InTypeID);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Memory")
	void SyncTransformToBuffer();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Memory")
	void SetCustomData(float InCustomData);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Memory")
	int32 GetBufferIndex() const { return BufferIndex; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Memory")
	FSBCompactEntityRecord GetCurrentRecord() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Memory")
	FSBRecordIndexUpdated OnRecordIndexUpdated;

	void UpdateBufferIndexInternal(int32 NewIndex);

private:
	void SyncTags();

	UPROPERTY()
	int32 BufferIndex;

	UPROPERTY()
	int32 EntityID;

	UPROPERTY()
	int32 TypeID;

	UPROPERTY()
	float CustomData;

	UPROPERTY()
	TWeakObjectPtr<UActorComponent> CachedStateComp;
};
