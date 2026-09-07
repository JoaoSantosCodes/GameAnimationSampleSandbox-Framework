// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBImmunityTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBImmuneSystemComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBInfectionStageChanged, FName, PathogenID, ESBInfectionStage, OldStage, ESBInfectionStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBFeverTriggered, float, FeverOffset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBInfectionCured, FName, PathogenID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBImmunityAcquired, FName, PathogenID);

/**
 * Componente de sistema imunológico, infecções por patógenos, febre e anticorpos
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBImmuneSystemComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBImmuneSystemComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Immunity")
	void SetupImmuneSystem(float InitialImmunityStrength = 1.0f);

	// Exposure & Treatment
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Immunity")
	bool ExposeToPathogen(const FSBPathogenStrain& Strain, float InitialLoad = 10.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Immunity")
	void ApplyMedicalTreatment(FName PathogenID, float MedicinePower);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Immunity")
	void SetImmunityModifier(float Multiplier);

	// Simulation Tick
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Immunity")
	void SimulateImmuneTick(float DeltaTime);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Immunity")
	FSBImmuneSystemData GetImmuneData() const { return ImmuneData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Immunity")
	bool IsInfectedWith(FName PathogenID) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Immunity")
	ESBInfectionStage GetInfectionStage(FName PathogenID) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Immunity")
	float GetTotalFeverOffset() const { return ImmuneData.BodyFeverOffset; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Immunity")
	FSBInfectionStageChanged OnInfectionStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Immunity")
	FSBFeverTriggered OnFeverTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Immunity")
	FSBInfectionCured OnInfectionCured;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Immunity")
	FSBImmunityAcquired OnImmunityAcquired;

private:
	void SyncTags();

	UPROPERTY()
	FSBImmuneSystemData ImmuneData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
