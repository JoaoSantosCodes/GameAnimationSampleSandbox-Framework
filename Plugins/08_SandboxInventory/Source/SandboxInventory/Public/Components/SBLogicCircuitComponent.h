#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBLogicCircuitTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBLogicCircuitComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBLogicConditionEvaluated, FName, NodeId, bool, bPassed, float, OutputValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBLogicSignalEmitted, ESBLogicWireColor, Wire, FName, Channel, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBLogicLatchToggled, FName, NodeId, bool, bNewState);

/**
 * Componente de automação lógica programável, portas booleanas, comparadores, processadores aritméticos e redes de sinais
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBLogicCircuitComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBLogicCircuitComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup APIs
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Logic")
	void SetupComparator(FName InNodeId, FName InChannelA, ESBLogicComparisonOp InOp, float InConstantVal, FName InOutChannel);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Logic")
	void SetupLogicGate(FName InNodeId, ESBLogicNodeType InGateType, FName InChannelA, FName InChannelB, FName InOutChannel);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Logic")
	void SetupArithmeticProcessor(FName InNodeId, FName InChannelA, ESBLogicArithmeticOp InOp, float InConstantVal, FName InOutChannel);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Logic")
	void SetupRSLatch(FName InNodeId, FName InSetChannel, FName InResetChannel, FName InOutChannel);

	// Signal Bus APIs
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Logic")
	void InjectSignal(ESBLogicWireColor Wire, FName Channel, float Value);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Logic")
	float ReadSignal(ESBLogicWireColor Wire, FName Channel) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Logic")
	void ClearSignals(ESBLogicWireColor Wire);

	// Simulation Tick
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Logic")
	void EvaluateCircuitTick(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Logic")
	FSBLogicGateData GetLogicGateData() const { return GateData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Logic")
	bool IsConditionMet() const { return GateData.bConditionEvaluatedTrue; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Logic")
	FSBLogicCircuitSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Logic")
	FSBLogicConditionEvaluated OnLogicConditionEvaluated;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Logic")
	FSBLogicSignalEmitted OnLogicSignalEmitted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Logic")
	FSBLogicLatchToggled OnLogicLatchToggled;

private:
	void SyncTags();
	float GetCombinedChannelValue(FName Channel) const;

	UPROPERTY()
	FSBLogicGateData GateData;

	TMap<FName, float> RedWireBus;
	TMap<FName, float> GreenWireBus;
	TMap<FName, float> CopperWireBus;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
