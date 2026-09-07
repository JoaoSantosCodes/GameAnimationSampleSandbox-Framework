// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBStressTestTypes.h"
#include "SBStressTestBotComponent.generated.h"

/**
 * Componente que transforma o ator em um agente de teste de estresse
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOXCORE_API USBStressTestBotComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBStressTestBotComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	/** Executa uma ação simulada e sincroniza com o subsistema */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|StressTest")
	void ExecuteSimulatedAction(ESBBotSimAction Action, bool bSuccess = true);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|StressTest")
	int32 BotId = 0;

	UFUNCTION(BlueprintPure, Category = "Sandbox|StressTest")
	ESBBotSimAction GetCurrentAction() const { return CurrentSimAction; }

private:
	ESBBotSimAction CurrentSimAction = ESBBotSimAction::Idle;
	void SyncTags();
};
