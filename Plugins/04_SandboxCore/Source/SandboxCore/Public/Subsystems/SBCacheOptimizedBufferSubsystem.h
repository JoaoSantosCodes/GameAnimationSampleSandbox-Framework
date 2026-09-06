#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBCacheTypes.h"
#include "SBCacheOptimizedBufferSubsystem.generated.h"

/**
 * Subsistema de gerenciamento de buffers contíguos com remoção O(1) swap-and-pop e zero alocações
 */
UCLASS()
class SANDBOXCORE_API USBCacheOptimizedBufferSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Buffer Operations
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Memory")
	void PreallocateBuffer(int32 MaxCapacity);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Memory")
	int32 InsertRecord(const FSBCompactEntityRecord& Record);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Memory")
	bool UpdateRecord(int32 RecordIndex, const FSBCompactEntityRecord& UpdatedRecord);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Memory")
	bool RemoveRecord(int32 RecordIndex, int32& OutSwappedEntityID);

	// Processing & Queries
	void ProcessRecordsZeroAlloc(TFunctionRef<void(const FSBCompactEntityRecord&)> Predicate) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Memory")
	FSBMemoryMetrics GetMemoryMetrics() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Memory")
	int32 GetRecordCount() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Memory")
	FSBCompactEntityRecord GetRecord(int32 Index) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Memory")
	int32 FindRecordIndexByEntityID(int32 EntityID) const;

private:
	UPROPERTY()
	TArray<FSBCompactEntityRecord> ContiguousBuffer;

	UPROPERTY()
	int32 PreallocatedCapacity;
};
