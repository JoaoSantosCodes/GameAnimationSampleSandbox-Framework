// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBPowerGridTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBPowerGridComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBPowerGridStateChanged, ESBPowerGridState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBPowerFlowChanged, float, TotalProduction, float, TotalDemand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBBreakerTripped, bool, bTripped);

/**
 * Componente de rede elétrica, geradores, baterias e consumidores industriais
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBPowerGridComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBPowerGridComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Power Grid API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Power")
	void SetupNode(ESBPowerNodeType Type, float Production, float Consumption, float BatteryCap);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Power")
	bool ConnectToPowerNode(USBPowerGridComponent* TargetNode);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Power")
	void DisconnectFromPowerNode(USBPowerGridComponent* TargetNode);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Power")
	void SetBreakerTripped(bool bTripped);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Power")
	void SimulatePowerGridTick(float DeltaTime, TArray<USBPowerGridComponent*>& Visited);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Power")
	void GetConnectedSubnet(TArray<USBPowerGridComponent*>& OutSubnet);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Power")
	FSBPowerNodeData GetNodeData() const { return NodeData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Power")
	ESBPowerGridState GetGridState() const { return NodeData.GridState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Power")
	bool IsPowered() const { return NodeData.GridState == ESBPowerGridState::Powered || NodeData.GridState == ESBPowerGridState::Charging || NodeData.GridState == ESBPowerGridState::Discharging; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Building|Power")
	FSBPowerGridSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Power")
	FSBPowerGridStateChanged OnPowerGridStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Power")
	FSBPowerFlowChanged OnPowerFlowChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Power")
	FSBBreakerTripped OnBreakerTripped;

private:
	void SyncPowerTags();

	UPROPERTY()
	FSBPowerNodeData NodeData;

	UPROPERTY()
	TArray<TWeakObjectPtr<USBPowerGridComponent>> ConnectedGridNodes;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
