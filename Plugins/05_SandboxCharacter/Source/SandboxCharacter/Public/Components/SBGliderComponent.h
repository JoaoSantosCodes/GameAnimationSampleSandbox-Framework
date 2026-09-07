// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBGliderTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBGliderComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnGliderStateChanged, ESBGliderState /*NewState*/);
DECLARE_MULTICAST_DELEGATE(FSBOnGliderEmergencyRetract);

/**
 * Componente de controle de planador, paraquedas, mergulho aéreo e física de voo
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBGliderComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBGliderComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Glider API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Glider")
	bool DeployGlider();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Glider")
	bool RetractGlider();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Glider")
	bool StartAerialDive();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Glider")
	bool StopAerialDive();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Glider")
	void UpdateFlightPhysics(float DeltaTime, float PitchInput);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Glider")
	bool IsGliding() const { return GliderState == ESBGliderState::Gliding || GliderState == ESBGliderState::Diving; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Glider")
	bool IsDiving() const { return GliderState == ESBGliderState::Diving; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Glider")
	ESBGliderState GetGliderState() const { return GliderState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Glider")
	FSBGliderFlightData GetFlightData() const { return FlightData; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Glider")
	FSBGliderSettings Settings;

	// Delegates
	FSBOnGliderStateChanged OnGliderStateChanged;
	FSBOnGliderEmergencyRetract OnGliderEmergencyRetract;

private:
	void SyncStateTags();

	UPROPERTY()
	ESBGliderState GliderState = ESBGliderState::Retracted;

	UPROPERTY()
	FSBGliderFlightData FlightData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
