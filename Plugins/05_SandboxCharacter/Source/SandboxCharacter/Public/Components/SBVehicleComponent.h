// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBVehicleTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBVehicleComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FSBOnVehicleOccupantChanged, AActor* /*Occupant*/, ESBVehicleSeat /*Seat*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnVehicleEngineStateChanged, bool /*bIsRunning*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnVehicleFuelChanged, float /*CurrentFuel*/);

/**
 * Componente de controle de veículos terrestres, trens de força, assentos e consumo de combustível
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBVehicleComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBVehicleComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Vehicle API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Vehicle")
	bool StartEngine();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Vehicle")
	bool StopEngine();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Vehicle")
	bool EnterVehicle(AActor* InActor, ESBVehicleSeat InSeat = ESBVehicleSeat::Driver);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Vehicle")
	bool ExitVehicle(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Vehicle")
	void SetThrottleInput(float InThrottle);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Vehicle")
	void SetSteeringInput(float InSteering);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Vehicle")
	void SetHandbrake(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Vehicle")
	void UpdateDrivetrainPhysics(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Vehicle")
	bool IsDriver(const AActor* InActor) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Vehicle")
	AActor* GetDriver() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Vehicle")
	bool IsOccupied() const { return Occupants.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Vehicle")
	bool IsSeatOccupied(ESBVehicleSeat InSeat) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Vehicle")
	FSBVehicleDrivetrainData GetDrivetrainData() const { return DrivetrainData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Vehicle")
	TArray<FSBVehicleSeatOccupant> GetOccupants() const { return Occupants; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Vehicle")
	FSBVehicleSettings Settings;

	// Delegates
	FSBOnVehicleOccupantChanged OnVehicleOccupantChanged;
	FSBOnVehicleEngineStateChanged OnVehicleEngineStateChanged;
	FSBOnVehicleFuelChanged OnVehicleFuelChanged;

private:
	void SyncDrivetrainTags();

	UPROPERTY()
	FSBVehicleDrivetrainData DrivetrainData;

	UPROPERTY()
	TArray<FSBVehicleSeatOccupant> Occupants;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
