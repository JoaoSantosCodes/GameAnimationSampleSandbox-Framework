// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBMachineryTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBMachineryComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBMachineryStateChanged, ESBMachineryState /*NewState*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSBMachineryPressureChanged, float /*CurrentPressure*/, float /*MaxPressure*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBMachineryOperatorChanged, AActor* /*Operator*/);

/**
 * Componente de controle de maquinário pesado, guindastes, escavadeiras e dinâmica hidráulica
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBMachineryComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBMachineryComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Machinery API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	bool EnterMachinery(AActor* InOperator);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	bool ExitMachinery(AActor* InOperator);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	void StartHydraulicPump(bool bStart);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	void SetBoomInput(float Input);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	void SetArmInput(float Input);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	void SetBucketInput(float Input);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	void SetSlewInput(float Input);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	void SetWinchInput(float Input);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	void SetOutriggersDeployed(bool bDeploy);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	bool AttachPayload(float PayloadMass);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	void DetachPayload();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	bool TriggerExcavateAction();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Machinery")
	void UpdateHydraulicPhysics(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Machinery")
	bool IsOperator(const AActor* InOperator) const { return CurrentOperator.IsValid() && CurrentOperator.Get() == InOperator; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Machinery")
	AActor* GetOperator() const { return CurrentOperator.Get(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Machinery")
	FSBMachineryHydraulicData GetHydraulicData() const { return HydraulicData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Machinery")
	ESBMachineryState GetMachineryState() const { return MachineryState; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Machinery")
	FSBMachinerySettings Settings;

	// Delegates
	FSBMachineryStateChanged OnMachineryStateChanged;
	FSBMachineryPressureChanged OnMachineryPressureChanged;
	FSBMachineryOperatorChanged OnMachineryOperatorChanged;

private:
	void SyncMachineryTags();

	UPROPERTY()
	ESBMachineryState MachineryState = ESBMachineryState::Parked;

	UPROPERTY()
	FSBMachineryHydraulicData HydraulicData;

	UPROPERTY()
	float BoomInput = 0.0f;

	UPROPERTY()
	float ArmInput = 0.0f;

	UPROPERTY()
	float BucketInput = 0.0f;

	UPROPERTY()
	float SlewInput = 0.0f;

	UPROPERTY()
	float WinchInput = 0.0f;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentOperator;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
