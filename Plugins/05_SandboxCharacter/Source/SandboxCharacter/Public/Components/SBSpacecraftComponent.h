// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBSpacecraftTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBSpacecraftComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnSpaceflightStateChanged, ESBSpaceflightState /*NewState*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnFlightAssistChanged, bool /*bIsActive*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnSpacecraftPilotChanged, AActor* /*Pilot*/);

/**
 * Componente de controle de espaçonaves, manobras orbitais e dinâmica 6-DOF em gravidade zero
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBSpacecraftComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBSpacecraftComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Spacecraft API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Spacecraft")
	bool EnterSpacecraft(AActor* InPilot);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Spacecraft")
	bool ExitSpacecraft(AActor* InPilot);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Spacecraft")
	void SetTranslationInput(const FVector& InTranslation);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Spacecraft")
	void SetRotationInput(const FVector& InRotation);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Spacecraft")
	void SetFlightAssist(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Spacecraft")
	void SetBoostActive(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Spacecraft")
	void SetAtmosphericReentry(bool bInReentry);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Spacecraft")
	void UpdateSpaceflightPhysics(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Spacecraft")
	bool IsPilot(const AActor* InPilot) const { return CurrentPilot.IsValid() && CurrentPilot.Get() == InPilot; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Spacecraft")
	AActor* GetPilot() const { return CurrentPilot.Get(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Spacecraft")
	bool IsFlightAssistActive() const { return FlightData.bFlightAssistActive; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Spacecraft")
	FSBSpacecraftFlightData GetFlightData() const { return FlightData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Spacecraft")
	ESBSpaceflightState GetFlightState() const { return FlightState; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Spacecraft")
	FSBSpacecraftSettings Settings;

	// Delegates
	FSBOnSpaceflightStateChanged OnSpaceflightStateChanged;
	FSBOnFlightAssistChanged OnFlightAssistChanged;
	FSBOnSpacecraftPilotChanged OnSpacecraftPilotChanged;

private:
	void SyncFlightTags();

	UPROPERTY()
	ESBSpaceflightState FlightState = ESBSpaceflightState::Docked;

	UPROPERTY()
	FSBSpacecraftFlightData FlightData;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentPilot;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
