#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBRadiationTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBRadiationExposureComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBRadiationStageChanged, ESBRadiationSicknessStage, OldStage, ESBRadiationSicknessStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBAcuteRadiationSicknessTriggered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBGeigerClick, float, FrequencyHz);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBDecontaminationCompleted, float, ClearedDose);

/**
 * Componente de dosimetria nuclear, ARS, contadores Geiger acústicos e blindagem de chumbo
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBRadiationExposureComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBRadiationExposureComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup & Medical Controls
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Radiation")
	void SetupRadiationComponent(float InitialDose_mSv = 0.0f, float InitialShielding = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Radiation")
	void EquipLeadShielding(float ShieldingFactor = 0.8f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Radiation")
	void AdministerAntiradMedication(float DoseReductionAmount_mSv = 1000.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Radiation")
	void PerformDecontamination(float SurfaceWashEfficiency = 0.8f);

	// Simulation
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Radiation")
	void SimulateRadiationTick(float DeltaTime, const FSBRadiationEnvironmentData& Environment);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Radiation")
	FSBRadiationExposureData GetRadiationData() const { return ExposureData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Radiation")
	ESBRadiationSicknessStage GetSicknessStage() const { return ExposureData.SicknessStage; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Radiation")
	bool IsGeigerClicking() const { return ExposureData.bIsGeigerClicking; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Radiation")
	FSBRadiationStageChanged OnRadiationStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Radiation")
	FSBAcuteRadiationSicknessTriggered OnAcuteRadiationSicknessTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Radiation")
	FSBGeigerClick OnGeigerClick;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Radiation")
	FSBDecontaminationCompleted OnDecontaminationCompleted;

private:
	void SyncTags();

	UPROPERTY()
	FSBRadiationExposureData ExposureData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
