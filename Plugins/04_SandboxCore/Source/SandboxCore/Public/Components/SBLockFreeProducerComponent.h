// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBLockFreeTypes.h"
#include "SBLockFreeProducerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBOnLockFreeEventProduced, int32, EventID, int32, EventType, float, Value);

/**
 * Componente produtor para envio seguro de eventos concorrentes à fila lock-free
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOXCORE_API USBLockFreeProducerComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBLockFreeProducerComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	/** Emite um evento concorrente para o subsistema lock-free */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LockFree")
	bool ProduceEvent(int32 EventType, float PayloadFloat);

	/** Retorna o total de eventos produzidos por este componente */
	UFUNCTION(BlueprintPure, Category = "Sandbox|LockFree")
	int32 GetProducedCount() const { return ProducedCount; }

	/** Delegate disparado quando um evento é produzido com sucesso */
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|LockFree")
	FSBOnLockFreeEventProduced OnEventProduced;

	/** Identificador numérico da entidade */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|LockFree")
	int32 EntityID = 1;

private:
	int32 ProducedCount = 0;
	int32 NextEventID = 1;

	void SyncTags();
};
