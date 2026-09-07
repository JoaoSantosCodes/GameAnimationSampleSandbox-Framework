// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBLockFreeTypes.h"
#include "Templates/Atomic.h"
#include "HAL/CriticalSection.h"
#include "SBLockFreeEventSubsystem.generated.h"

/**
 * Subsistema de alta performance para gerenciamento de filas circulares de eventos e mensageria
 */
UCLASS()
class SANDBOXCORE_API USBLockFreeEventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBLockFreeEventSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Inicializa ou redimensiona a capacidade do buffer circular */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LockFree")
	void InitializeQueue(int32 InCapacity = 1024);

	/** Enfileira evento de forma thread-safe */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LockFree")
	bool EnqueueEvent(const FSBLockFreeEvent& InEvent);

	/** Desenfileira um único evento em ordem FIFO */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LockFree")
	bool DequeueEvent(FSBLockFreeEvent& OutEvent);

	/** Drena até MaxToDrain eventos para processamento em lote */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LockFree")
	int32 DrainEvents(TArray<FSBLockFreeEvent>& OutEvents, int32 MaxToDrain = 100);

	/** Retorna as métricas consolidadas da fila */
	UFUNCTION(BlueprintPure, Category = "Sandbox|LockFree")
	FSBLockFreeQueueMetrics GetMetrics() const;

	/** Retorna quantidade de eventos pendentes na fila */
	UFUNCTION(BlueprintPure, Category = "Sandbox|LockFree")
	int32 GetPendingCount() const;

	/** Limpa a fila e redefine ponteiros */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LockFree")
	void ClearQueue();

private:
	TArray<FSBLockFreeEvent> RingBuffer;
	int32 Capacity = 1024;

	TAtomic<int32> HeadIndex{0};
	TAtomic<int32> TailIndex{0};

	TAtomic<int32> EnqueuedCount{0};
	TAtomic<int32> DequeuedCount{0};
	TAtomic<int32> DroppedCount{0};
	TAtomic<int32> PendingCount{0};

	mutable FCriticalSection QueueLock;
};
