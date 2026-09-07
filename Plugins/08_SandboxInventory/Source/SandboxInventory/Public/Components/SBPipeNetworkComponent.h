// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBPipeNetworkTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBPipeNetworkComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBPipeFlowStateChanged, ESBPipeFlowState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBFluidPressureChanged, float, CurrentPressure, float, MaxSafePressure);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBFluidLevelChanged, float, CurrentVolume, float, MaxCapacity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBPipeRupture, ESBFluidType, FluidType, float, LostVolume);

/**
 * Componente de redes de tubulação, fluidos, gases, bombas e tanques
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBPipeNetworkComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBPipeNetworkComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Pipe Network API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Pipes")
	void SetupNode(ESBPipeNodeType InNodeType, ESBFluidType InFluidType, float InCapacity, float InMaxPressure);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Pipes")
	void SetPumpActive(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Pipes")
	void SetValveOpenPercentage(float InOpenPct);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Pipes")
	bool ConnectPipe(USBPipeNetworkComponent* TargetPipe);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Pipes")
	void DisconnectPipe(USBPipeNetworkComponent* TargetPipe);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Pipes")
	float InjectFluid(ESBFluidType InType, float InAmount);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Pipes")
	float ExtractFluid(float InAmount);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Pipes")
	void SimulateFluidDynamicsTick(float DeltaTime, TArray<USBPipeNetworkComponent*>& Visited);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Pipes")
	void GetConnectedPipeNetwork(TArray<USBPipeNetworkComponent*>& OutNetwork);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Pipes")
	FSBPipeNodeData GetNodeData() const { return NodeData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Pipes")
	ESBPipeFlowState GetFlowState() const { return NodeData.FlowState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Pipes")
	bool IsFlowing() const { return NodeData.FlowState == ESBPipeFlowState::Flowing; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Building|Pipes")
	FSBPipeNetworkSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Pipes")
	FSBPipeFlowStateChanged OnPipeFlowStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Pipes")
	FSBFluidPressureChanged OnFluidPressureChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Pipes")
	FSBFluidLevelChanged OnFluidLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Pipes")
	FSBPipeRupture OnPipeRupture;

private:
	void SyncFluidTags();

	UPROPERTY()
	FSBPipeNodeData NodeData;

	UPROPERTY()
	TArray<TWeakObjectPtr<USBPipeNetworkComponent>> ConnectedPipes;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
