// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBAsyncSerializationTypes.h"
#include "SBAsyncSerializationSubsystem.generated.h"

/**
 * Subsistema assíncrono de particionamento em chunks e serialização binária com integridade hash
 */
UCLASS()
class SANDBOXCORE_API USBAsyncSerializationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBAsyncSerializationSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Serializa um conjunto de registros em um slot de memória de forma síncrona com validação de hash */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Serialization")
	int32 SerializeSnapshotSync(const TArray<FSBAsyncSaveRecord>& InRecords, FName SlotName);

	/** Desserializa registros de um slot validando a integridade dos hashes de cada registro */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Serialization")
	bool DeserializeSnapshotSync(FName SlotName, TArray<FSBAsyncSaveRecord>& OutRecords);

	/** Utilitário de cálculo de hash MD5 determinístico para payloads binários */
	UFUNCTION(BlueprintPure, Category = "Sandbox|Serialization")
	static FString ComputePayloadHash(const TArray<uint8>& BinaryData);

	/** Retorna as métricas operacionais */
	UFUNCTION(BlueprintPure, Category = "Sandbox|Serialization")
	FSBAsyncSaveMetrics GetMetrics() const;

	/** Retorna se um determinado slot existe */
	UFUNCTION(BlueprintPure, Category = "Sandbox|Serialization")
	bool HasSlot(FName SlotName) const;

	/** Limpa um slot específico */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Serialization")
	void ClearSlot(FName SlotName);

	/** Limpa todos os slots e métricas */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Serialization")
	void ResetSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Serialization")
	FSBOnAsyncSaveCompleted OnAsyncSaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Serialization")
	FSBOnAsyncLoadCompleted OnAsyncLoadCompleted;

private:
	TMap<FName, FSBAsyncSaveChunk> SavedSlots;

	int32 TotalSavesCompleted = 0;
	int32 TotalLoadsCompleted = 0;
	int32 TotalBytesSerialized = 0;
	int32 TotalChunksProcessed = 0;
};
