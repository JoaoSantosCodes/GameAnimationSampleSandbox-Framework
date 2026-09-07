// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBAtmosphereTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBAtmosphericSafetyComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBHypoxiaStateChanged, bool, bIsHypoxic);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBToxicInhalationTriggered, float, ToxicityLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBSuitBreached);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBFilterExhausted);

/**
 * Componente de controle atmosférico, depleção de O2, hipóxia, trajes espaciais e gases tóxicos
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBAtmosphericSafetyComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBAtmosphericSafetyComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Equipment & Maintenance
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Atmosphere")
	void SetupAtmosphericSafety(float InitialOxygen = 100.0f, float InitialFilter = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Atmosphere")
	void ToggleSuitSeal(bool bSealSuit);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Atmosphere")
	void RefillOxygenReserve(float Amount = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Atmosphere")
	void ReplaceFilter(float NewFilterIntegrity = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Atmosphere")
	void PatchSuitLeak(float SealRepairAmount = 100.0f);

	// Simulation
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Atmosphere")
	void SimulateAtmosphereTick(float DeltaTime, const FSBAtmosphereEnvironmentData& Environment);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Atmosphere")
	FSBAtmosphericSafetyData GetAtmosphericSafetyData() const { return SafetyData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Atmosphere")
	bool IsHypoxic() const { return SafetyData.bIsHypoxic; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Atmosphere")
	bool IsInToxicInhalation() const { return SafetyData.bInToxicInhalation; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Atmosphere")
	FSBHypoxiaStateChanged OnHypoxiaStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Atmosphere")
	FSBToxicInhalationTriggered OnToxicInhalationTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Atmosphere")
	FSBSuitBreached OnSuitBreached;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Atmosphere")
	FSBFilterExhausted OnFilterExhausted;

private:
	void SyncTags();

	UPROPERTY()
	FSBAtmosphericSafetyData SafetyData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
