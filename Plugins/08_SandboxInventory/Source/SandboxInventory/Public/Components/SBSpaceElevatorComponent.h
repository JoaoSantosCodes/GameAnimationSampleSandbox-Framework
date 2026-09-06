#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBSpaceElevatorTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBSpaceElevatorComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBSpaceElevatorStateChanged, int32, PhaseIndex, ESBSpaceElevatorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBSpaceElevatorPhaseCompleted, int32, CompletedPhaseIndex, FName, PhaseName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBSpaceElevatorItemDeposited, int32, PhaseIndex, FName, ItemId, int32, DepositedAmount);

/**
 * Componente de megaestrutura do Elevador Espacial Modular, despacho orbital e avanço de patamares planetários
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBSpaceElevatorComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBSpaceElevatorComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup APIs
	UFUNCTION(BlueprintCallable, Category = "Sandbox|SpaceElevator")
	void SetupSpaceElevator(int32 TotalPhases, float PowerRequiredMW);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|SpaceElevator")
	void ConfigurePhaseRequirement(int32 PhaseIndex, FName PhaseName, const TMap<FName, int32>& RequiredItems);

	// Cargo Deposit APIs
	UFUNCTION(BlueprintCallable, Category = "Sandbox|SpaceElevator")
	int32 DepositPhaseItem(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintPure, Category = "Sandbox|SpaceElevator")
	int32 GetDepositedItemCount(int32 PhaseIndex, FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|SpaceElevator")
	int32 GetRequiredItemCount(int32 PhaseIndex, FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|SpaceElevator")
	bool IsCurrentPhaseRequirementMet() const;

	// Launch Dispatch APIs
	UFUNCTION(BlueprintCallable, Category = "Sandbox|SpaceElevator")
	bool LaunchOrbitalDelivery();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|SpaceElevator")
	void SetPowerSupplied(bool bSupplied);

	// Simulation Tick
	UFUNCTION(BlueprintCallable, Category = "Sandbox|SpaceElevator")
	void SimulateElevatorTick(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|SpaceElevator")
	FSBSpaceElevatorData GetSpaceElevatorData() const { return ElevatorData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|SpaceElevator")
	ESBSpaceElevatorState GetElevatorState() const { return ElevatorData.State; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|SpaceElevator")
	FSBSpaceElevatorSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|SpaceElevator")
	FSBSpaceElevatorStateChanged OnSpaceElevatorStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|SpaceElevator")
	FSBSpaceElevatorPhaseCompleted OnSpaceElevatorPhaseCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|SpaceElevator")
	FSBSpaceElevatorItemDeposited OnSpaceElevatorItemDeposited;

private:
	void SyncTags();
	FSBSpaceElevatorPhaseRequirement* GetCurrentPhaseRequirement();
	const FSBSpaceElevatorPhaseRequirement* GetCurrentPhaseRequirement() const;

	UPROPERTY()
	FSBSpaceElevatorData ElevatorData;

	float StateTimer = 0.0f;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
