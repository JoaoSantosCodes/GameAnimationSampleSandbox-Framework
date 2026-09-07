// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBTraumaTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBTraumaInjuryComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBLimbFractured, ESBBodyLimb, Limb);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBBleedStateChanged, ESBBodyLimb, Limb, ESBBleedType, BleedType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBHypovolemicShockTriggered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBHypovolemicShockRecovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBTourniquetStateChanged, ESBBodyLimb, Limb, bool, bApplied);

/**
 * Componente de trauma físico, fraturas ósseas, hemorragias e torniquetes
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBTraumaInjuryComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBTraumaInjuryComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Trauma")
	void SetupTraumaSystem(float InitialBloodVolume = 5.0f);

	// Damage & First Aid
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Trauma")
	void InflictLimbDamage(ESBBodyLimb Limb, float Damage, bool bCanFracture = false, ESBBleedType Bleed = ESBBleedType::None);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Trauma")
	bool ApplySplint(ESBBodyLimb Limb);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Trauma")
	bool ApplyTourniquet(ESBBodyLimb Limb);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Trauma")
	bool RemoveTourniquet(ESBBodyLimb Limb);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Trauma")
	void ApplyBandageOrSuture(ESBBodyLimb Limb);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Trauma")
	void TransfuseBlood(float VolumeLiters);

	// Simulation Tick
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Trauma")
	void SimulateTraumaTick(float DeltaTime);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Trauma")
	FSBTraumaSystemData GetTraumaData() const { return TraumaData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Trauma")
	FSBLimbTrauma GetLimbData(ESBBodyLimb Limb) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Trauma")
	bool IsLimbFractured(ESBBodyLimb Limb) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Trauma")
	bool IsBleeding() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Trauma")
	bool IsArterialBleeding() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Trauma")
	FSBLimbFractured OnLimbFractured;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Trauma")
	FSBBleedStateChanged OnBleedStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Trauma")
	FSBHypovolemicShockTriggered OnHypovolemicShockTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Trauma")
	FSBHypovolemicShockRecovered OnHypovolemicShockRecovered;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Trauma")
	FSBTourniquetStateChanged OnTourniquetStateChanged;

private:
	void InitDefaultLimbs();
	void SyncTags();

	UPROPERTY()
	FSBTraumaSystemData TraumaData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
