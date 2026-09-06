#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBStressTestTypes.h"
#include "SBStressTestSubsystem.generated.h"

/**
 * Subsistema de estresse automatizado e gerenciamento de enxame de agentes simulados
 */
UCLASS()
class SANDBOXCORE_API USBStressTestSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBStressTestSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Instancia e registra um enxame de bots no subsistema */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|StressTest")
	int32 SpawnBotSwarm(int32 BotCount);

	/** Executa um ciclo de tick de estresse avançando simulações de todos os agentes */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|StressTest")
	void ExecuteStressTick(float DeltaTime, int32 ActionCyclesPerBot);

	/** Registra uma ação executada por um agente específico */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|StressTest")
	void RecordBotAction(int32 BotIndex, ESBBotSimAction Action, bool bSuccess);

	/** Consulta o estado de um bot específico */
	UFUNCTION(BlueprintPure, Category = "Sandbox|StressTest")
	FSBBotSimAgentState GetBotState(int32 BotIndex) const;

	/** Retorna as métricas consolidadas de estresse */
	UFUNCTION(BlueprintPure, Category = "Sandbox|StressTest")
	FSBStressTestMetrics GetMetrics() const { return Metrics; }

	/** Reinicializa o subsistema e limpa bots */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|StressTest")
	void ResetSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|StressTest")
	FSBOnStressTestCycleCompleted OnStressTestCycleCompleted;

private:
	TMap<int32, FSBBotSimAgentState> SwarmBots;
	FSBStressTestMetrics Metrics;
};
