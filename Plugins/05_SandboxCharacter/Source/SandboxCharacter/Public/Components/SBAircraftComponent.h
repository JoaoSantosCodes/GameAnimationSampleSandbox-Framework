#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBAircraftTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBAircraftComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnFlightStateChanged, ESBFlightState /*NewState*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnAircraftStallStateChanged, bool /*bIsStalling*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnAircraftPilotChanged, AActor* /*Pilot*/);

/**
 * Componente de controle e física aerodinâmica para aviões, jatos e helicópteros
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBAircraftComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBAircraftComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Aircraft API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Aircraft")
	bool EnterAircraft(AActor* InPilot);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Aircraft")
	bool ExitAircraft(AActor* InPilot);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Aircraft")
	void SetThrottleInput(float InThrottle);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Aircraft")
	void SetFlightControls(float InPitch, float InRoll, float InYaw);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Aircraft")
	void SetVTOLMode(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Aircraft")
	void UpdateFlightPhysics(float DeltaTime, const FVector& CurrentActorLocation);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Aircraft")
	bool IsPilot(const AActor* InPilot) const { return CurrentPilot.IsValid() && CurrentPilot.Get() == InPilot; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Aircraft")
	AActor* GetPilot() const { return CurrentPilot.Get(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Aircraft")
	bool IsAirborne() const { return FlightData.bIsAirborne; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Aircraft")
	bool IsStalling() const { return FlightData.bIsStalling; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Aircraft")
	FSBAircraftFlightData GetFlightData() const { return FlightData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Aircraft")
	ESBFlightState GetFlightState() const { return FlightState; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Aircraft")
	FSBAircraftSettings Settings;

	// Delegates
	FSBOnFlightStateChanged OnFlightStateChanged;
	FSBOnAircraftStallStateChanged OnAircraftStallStateChanged;
	FSBOnAircraftPilotChanged OnAircraftPilotChanged;

private:
	void SyncFlightTags();

	UPROPERTY()
	ESBFlightState FlightState = ESBFlightState::Parked;

	UPROPERTY()
	FSBAircraftFlightData FlightData;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentPilot;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
