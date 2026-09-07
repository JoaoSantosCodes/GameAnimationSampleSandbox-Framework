// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBHerbologyTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBAfflictionComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBAfflictionApplied, ESBAfflictionType, Type, float, Severity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBAfflictionNeutralized, ESBAfflictionType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBMotorImpairmentStateChanged, bool, bImpaired);

/**
 * Componente de aflições, comprometimento motor, elixires neutralizadores e resistência por inoculação
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBAfflictionComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBAfflictionComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup & Affliction Controls
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Afflictions")
	void SetupAfflictionComponent(float InitialResistance = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Afflictions")
	void ApplyAffliction(ESBAfflictionType Type, float Severity, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Afflictions")
	void ApplyNeutralizer(ESBNeutralizerType Neutralizer, float Potency = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Afflictions")
	void ApplyInoculation(float ResistanceBoost = 0.5f, float Duration = 60.0f);

	// Simulation
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Afflictions")
	void SimulateAfflictionTick(float DeltaTime);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Afflictions")
	FSBCharacterAfflictionState GetAfflictionState() const { return AfflictionState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Afflictions")
	bool IsMotorImpaired() const { return AfflictionState.bIsMotorImpaired; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Afflictions")
	float GetMotorSpeedMultiplier() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Afflictions")
	FSBAfflictionApplied OnAfflictionApplied;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Afflictions")
	FSBAfflictionNeutralized OnAfflictionNeutralized;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Afflictions")
	FSBMotorImpairmentStateChanged OnMotorImpairmentStateChanged;

private:
	void SyncTags();

	UPROPERTY()
	FSBCharacterAfflictionState AfflictionState;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
