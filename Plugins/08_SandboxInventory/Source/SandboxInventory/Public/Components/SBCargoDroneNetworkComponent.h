#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBCargoDroneTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBCargoDroneNetworkComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBDroneFlightStateChanged, FName, DroneId, ESBCargoDroneFlightState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBDronePortArrived, FName, DroneId, FName, PortId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBDroneCargoLoaded, FName, DroneId, FName, ItemId, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBDroneBatteryUpdated, FName, DroneId, float, CurrentBattery);

/**
 * Componente de logística aérea autônoma por drones de carga, docas de recarga e corredores aéreos 3D
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBCargoDroneNetworkComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBCargoDroneNetworkComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup APIs
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Drone")
	void SetupDrone(FName InDroneId, ESBCargoDroneModel InModel, float InFlightSpeed, float InMaxBattery, int32 InCapacity);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Drone")
	void RegisterDronePort(FName PortId, const FVector& Location, bool bHasRecharge);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Drone")
	void SetFlightRoute(FName OriginPortId, FName DestinationPortId);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Drone")
	bool DispatchDrone();

	// Cargo Management
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Drone")
	int32 LoadCargoIntoDrone(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Drone")
	int32 UnloadCargoFromDrone(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Drone")
	int32 GetDroneCargoCount(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Drone")
	int32 GetTotalCargoCount() const;

	// Simulation Tick
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Drone")
	void SimulateDroneTick(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Drone")
	FSBCargoDroneData GetDroneData() const { return DroneData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Drone")
	ESBCargoDroneFlightState GetFlightState() const { return DroneData.FlightState; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Drone")
	FSBCargoDroneSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Drone")
	FSBDroneFlightStateChanged OnDroneFlightStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Drone")
	FSBDronePortArrived OnDronePortArrived;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Drone")
	FSBDroneCargoLoaded OnDroneCargoLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Drone")
	FSBDroneBatteryUpdated OnDroneBatteryUpdated;

private:
	void SyncTags();

	UPROPERTY()
	FSBCargoDroneData DroneData;

	UPROPERTY()
	TMap<FName, FSBDronePortData> RegisteredPorts;

	float StateTimer = 0.0f;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
