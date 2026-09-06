#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBIndustrialTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBIndustrialProcessorComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBProcessorStateChanged, ESBProcessorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBProcessorCycleCompleted, FName, RecipeId, int32, TotalCycles);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBProcessorTemperatureChanged, float, CurrentTemp, float, MaxSafeTemp);

/**
 * Componente de maquinário industrial, fundições, refinarias, montadoras e fabricação automatizada
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBIndustrialProcessorComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBIndustrialProcessorComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Processor API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Industrial")
	void SetupProcessor(ESBProcessorType InType, const FSBIndustrialRecipe& InRecipe);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Industrial")
	void SetOverclockMultiplier(float InMultiplier);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Industrial")
	void SetPowerSupplied(bool bSupplied);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Industrial")
	int32 DepositInputItem(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Industrial")
	float DepositInputFluid(ESBFluidType FluidType, float Volume);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Industrial")
	int32 WithdrawOutputItem(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Industrial")
	float WithdrawOutputFluid(ESBFluidType FluidType, float Volume);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Industrial")
	int32 GetInputItemCount(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Industrial")
	int32 GetOutputItemCount(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Industrial")
	float GetInputFluidVolume(ESBFluidType FluidType) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Industrial")
	float GetOutputFluidVolume(ESBFluidType FluidType) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Industrial")
	void SimulateProcessorTick(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Industrial")
	FSBProcessorData GetProcessorData() const { return ProcessorData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Industrial")
	ESBProcessorState GetProcessorState() const { return ProcessorData.ProcessorState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Industrial")
	bool IsProcessing() const { return ProcessorData.ProcessorState == ESBProcessorState::Processing; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Industrial")
	FSBProcessorSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Industrial")
	FSBProcessorStateChanged OnProcessorStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Industrial")
	FSBProcessorCycleCompleted OnProcessorCycleCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Industrial")
	FSBProcessorTemperatureChanged OnProcessorTemperatureChanged;

private:
	void SyncProcessorState(ESBProcessorState NewState);
	void SyncTags();

	UPROPERTY()
	FSBProcessorData ProcessorData;

	TMap<FName, int32> InputItemBuffer;
	TMap<ESBFluidType, float> InputFluidBuffer;
	TMap<FName, int32> OutputItemBuffer;
	TMap<ESBFluidType, float> OutputFluidBuffer;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
