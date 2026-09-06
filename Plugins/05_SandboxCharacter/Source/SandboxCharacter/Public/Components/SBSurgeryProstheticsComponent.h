#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBSurgicalTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBSurgeryProstheticsComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBAnesthesiaStateChanged, bool, bAnesthetized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBSurgicalOperationCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBProstheticInstalled, ESBSurgicalLimbType, Limb, ESBProstheticGrade, Grade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBOrganRejectionWarning);

/**
 * Componente de cirurgias avançadas, anestesia, próteses biomecânicas e transplantes de órgãos
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBSurgeryProstheticsComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBSurgeryProstheticsComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup & Surgical Controls
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Surgery")
	void SetupSurgicalComponent(float InitialOrganHealth = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Surgery")
	void AdministerAnesthesia(float Duration = 30.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Surgery")
	void StartSurgicalOperation(float EstimatedDuration = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Surgery")
	void InstallProsthetic(ESBSurgicalLimbType Limb, ESBProstheticGrade Grade, float Efficiency = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Surgery")
	void PerformOrganTransplant(float RestoredHealth = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Surgery")
	void AdministerImmunosuppressant(float Potency = 1.0f);

	// Simulation
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Surgery")
	void SimulateSurgicalTick(float DeltaTime);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Surgery")
	FSBSurgicalPatientData GetPatientData() const { return PatientData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Surgery")
	bool IsUnderAnesthesia() const { return PatientData.bIsAnesthetized; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Surgery")
	bool HasProsthetic(ESBSurgicalLimbType Limb) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Surgery")
	float GetOverallEfficiencyBonus() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Surgery")
	FSBAnesthesiaStateChanged OnAnesthesiaStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Surgery")
	FSBSurgicalOperationCompleted OnSurgicalOperationCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Surgery")
	FSBProstheticInstalled OnProstheticInstalled;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Surgery")
	FSBOrganRejectionWarning OnOrganRejectionWarning;

private:
	void SyncTags();

	UPROPERTY()
	FSBSurgicalPatientData PatientData;

	UPROPERTY()
	float TargetOperationDuration;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
