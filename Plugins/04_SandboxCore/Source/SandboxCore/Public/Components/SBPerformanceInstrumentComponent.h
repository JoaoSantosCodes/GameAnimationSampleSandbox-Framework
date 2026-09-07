// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBProfilerTypes.h"
#include "SBPerformanceInstrumentComponent.generated.h"

/**
 * Componente para instrumentação de telemetria e custo de CPU/memória de um ator
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOXCORE_API USBPerformanceInstrumentComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBPerformanceInstrumentComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	/** Nome do componente ou subsistema sob observação */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|PerformanceProfiler")
	FName TrackedComponentName = TEXT("SBCombatComponent");

	/** Limite de orçamento em microsegundos */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|PerformanceProfiler")
	float BudgetThresholdUs = 100.0f;

	/** Registra uma medição de execução */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|PerformanceProfiler")
	void RecordExecution(float DurationUs, int64 MemoryBytes);

	/** Handler chamado ao estourar orçamento de desempenho */
	UFUNCTION()
	void HandleBudgetExceeded(FName InComponentName, float ActualDurationUs, float InBudgetThresholdUs);

	/** Retorna total de execuções registradas localmente */
	UFUNCTION(BlueprintPure, Category = "Sandbox|PerformanceProfiler")
	int32 GetLocalExecutionCount() const { return LocalExecutionCount; }

private:
	int32 LocalExecutionCount = 0;
	void SyncTags();
};
