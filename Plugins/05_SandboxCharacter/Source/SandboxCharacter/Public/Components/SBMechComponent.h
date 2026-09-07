// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBMechTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBMechComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnMechStateChanged, ESBMechState /*NewState*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnMechOverheatChanged, bool /*bIsOverheated*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnMechPilotChanged, AActor* /*Pilot*/);

/**
 * Componente de controle de mechas bípedes pesados, exoesqueletos e propulsores de salto
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBMechComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBMechComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Mech API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Mech")
	bool EnterMech(AActor* InPilot);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Mech")
	bool ExitMech(AActor* InPilot);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Mech")
	void SetPowerState(bool bPowerOn);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Mech")
	void SetMoveInput(const FVector& InMove);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Mech")
	void ActivateJumpJets(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Mech")
	bool TriggerDash();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Mech")
	void UpdateMechPhysics(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Mech")
	bool IsPilot(const AActor* InPilot) const { return CurrentPilot.IsValid() && CurrentPilot.Get() == InPilot; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Mech")
	AActor* GetPilot() const { return CurrentPilot.Get(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Mech")
	bool IsOverheated() const { return OperationalData.bIsOverheated; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Mech")
	FSBMechOperationalData GetOperationalData() const { return OperationalData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Mech")
	ESBMechState GetMechState() const { return MechState; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Mech")
	FSBMechSettings Settings;

	// Delegates
	FSBOnMechStateChanged OnMechStateChanged;
	FSBOnMechOverheatChanged OnMechOverheatChanged;
	FSBOnMechPilotChanged OnMechPilotChanged;

private:
	void SyncMechTags();

	UPROPERTY()
	ESBMechState MechState = ESBMechState::PoweredOff;

	UPROPERTY()
	FSBMechOperationalData OperationalData;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentPilot;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
