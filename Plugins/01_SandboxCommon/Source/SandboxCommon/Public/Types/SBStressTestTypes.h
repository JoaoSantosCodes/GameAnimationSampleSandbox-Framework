// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBStressTestTypes.generated.h"

/**
 * Ação simulada executada por um agente bot durante o estresse
 */
UENUM(BlueprintType)
enum class ESBBotSimAction : uint8
{
	Idle = 0,
	CombatMelee = 1,
	HarvestMining = 2,
	VehicleMount = 3,
	SurgeryProcedure = 4,
	CropFarming = 5
};

/**
 * Estado e telemetria de um agente individual no enxame de estresse
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBBotSimAgentState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	int32 BotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	ESBBotSimAction CurrentAction = ESBBotSimAction::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	int32 ActionsCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	int32 ErrorsEncountered = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	bool bIsActive = false;
};

/**
 * Métricas consolidadas do subsistema de testes de estresse
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBStressTestMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	int32 TotalSimulatedBots = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	int32 TotalActionsExecuted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	int32 TotalDeadlocksDetected = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	int32 TotalUnhandledExceptions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StressTest")
	float StressDurationSeconds = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnStressTestCycleCompleted, int32, TotalBots, int32, TotalActions);
