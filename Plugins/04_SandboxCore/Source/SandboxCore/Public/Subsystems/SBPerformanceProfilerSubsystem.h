// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBProfilerTypes.h"
#include "SBPerformanceProfilerSubsystem.generated.h"

/**
 * Subsistema de medição de desempenho e custos de tempo de execução por componente
 */
UCLASS()
class SANDBOXCORE_API USBPerformanceProfilerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBPerformanceProfilerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Registra uma amostra de medição de um componente */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|PerformanceProfiler")
	void RecordComponentSample(FName ComponentName, float DurationUs, int64 MemoryBytes);

	/** Define o limiar de orçamento em microsegundos para um componente */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|PerformanceProfiler")
	void SetBudgetThreshold(FName ComponentName, float BudgetThresholdUs);

	/** Consulta os dados estatísticos consolidados de um componente */
	UFUNCTION(BlueprintPure, Category = "Sandbox|PerformanceProfiler")
	FSBComponentSampleData GetComponentStats(FName ComponentName) const;

	/** Retorna lista de componentes que excederam o limiar especificado */
	UFUNCTION(BlueprintPure, Category = "Sandbox|PerformanceProfiler")
	TArray<FSBComponentSampleData> GetHotComponents(float ThresholdUs) const;

	/** Retorna as métricas consolidadas de profiling */
	UFUNCTION(BlueprintPure, Category = "Sandbox|PerformanceProfiler")
	FSBPerformanceProfilerMetrics GetMetrics() const;

	/** Reinicializa o subsistema */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|PerformanceProfiler")
	void ResetSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|PerformanceProfiler")
	FSBOnComponentBudgetExceeded OnComponentBudgetExceeded;

private:
	TMap<FName, FSBComponentSampleData> ComponentStatsMap;
	TMap<FName, float> BudgetThresholdsMap;
	int32 TotalSamplesRecorded = 0;
	float TotalTimeBudgetSpent = 0.0f;
};
