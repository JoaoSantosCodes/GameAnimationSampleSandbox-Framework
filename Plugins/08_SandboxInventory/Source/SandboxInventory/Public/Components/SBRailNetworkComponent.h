// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBRailTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBRailNetworkComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBTrainStateChanged, FName, TrainId, ESBTrainMovementState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBTrainStationArrived, FName, TrainId, FName, StationName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBTrainCargoTransferred, FName, TrainId, FName, ItemId, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBRailSignalChanged, int32, BlockId, ESBRailSignalState, NewSignal);

/**
 * Componente de malha ferroviária, sinalização de blocos, semáforos, composições de trem e estações automatizadas
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBRailNetworkComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBRailNetworkComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Rail Consist & Route API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	void SetupTrainConsist(FName InTrainId, float InMaxSpeed);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	void AddWagon(const FSBRailWagonData& InWagon);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	void AddStationToSchedule(FName InStationName);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	void StartTravel();

	// Block Signalling API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	void RegisterRailBlock(int32 InBlockId);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	void SetBlockOccupied(int32 InBlockId, bool bOccupied, FName InOccupyingTrainId);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	bool RequestBlockReservation(int32 InBlockId, FName InTrainId);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	void ReleaseBlockReservation(int32 InBlockId, FName InTrainId);

	// Cargo Transfer API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	int32 LoadCargoIntoWagon(int32 WagonIndex, FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	int32 UnloadCargoFromWagon(int32 WagonIndex, FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Rail")
	int32 GetWagonCargoCount(int32 WagonIndex, FName ItemId) const;

	// Simulation Tick
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Rail")
	void SimulateRailTick(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Rail")
	FSBTrainConsist GetTrainConsist() const { return TrainConsist; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Rail")
	ESBTrainMovementState GetMovementState() const { return TrainConsist.MovementState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Rail")
	bool IsTraveling() const { return TrainConsist.MovementState == ESBTrainMovementState::Traveling; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Rail")
	FSBRailNetworkSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Rail")
	FSBTrainStateChanged OnTrainStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Rail")
	FSBTrainStationArrived OnTrainStationArrived;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Rail")
	FSBTrainCargoTransferred OnTrainCargoTransferred;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Rail")
	FSBRailSignalChanged OnRailSignalChanged;

private:
	void SyncTrainState(ESBTrainMovementState NewState);
	void SyncTags();

	UPROPERTY()
	FSBTrainConsist TrainConsist;

	UPROPERTY()
	TMap<int32, FSBRailBlockData> RailBlocks;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
