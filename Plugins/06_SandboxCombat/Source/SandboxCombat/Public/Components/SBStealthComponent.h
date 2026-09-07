// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBStealthTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBStealthComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnStealthStateChanged, ESBStealthState, NewState, float, AlertPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnNoiseEmitted, const FSBNoiseEvent&, NoiseEvent);

/**
 * Componente para controle de furtividade, percepção, propagação de ruído acústico e medidor de alerta
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBStealthComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBStealthComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Stealth & Noise API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Stealth")
	FSBNoiseEvent EmitNoise(float Radius, float Loudness = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Stealth")
	ESBStealthState UpdateDetection(float BaseExposure, float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Stealth")
	float GetEffectiveVisibility() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Stealth")
	float GetAlertPercent() const { return CurrentAlertPercent; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Stealth")
	ESBStealthState GetStealthState() const { return CurrentStealthState; }

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Stealth")
	void SetCrouched(bool bInCrouched) { bIsCrouched = bInCrouched; }

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Stealth")
	void SetInShadows(bool bInShadows) { bIsInShadows = bInShadows; }

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Stealth")
	void ResetStealth();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Stealth")
	int32 GetNoiseHistoryCount() const { return NoiseHistory.Num(); }

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Combat|Stealth")
	FSBStealthSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Stealth")
	FSBOnStealthStateChanged OnStealthStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Stealth")
	FSBOnNoiseEmitted OnNoiseEmitted;

private:
	void SyncStateTags();

	UPROPERTY()
	ESBStealthState CurrentStealthState = ESBStealthState::Hidden;

	UPROPERTY()
	float CurrentAlertPercent = 0.0f;

	UPROPERTY()
	bool bIsCrouched = false;

	UPROPERTY()
	bool bIsInShadows = false;

	UPROPERTY()
	TArray<FSBNoiseEvent> NoiseHistory;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
