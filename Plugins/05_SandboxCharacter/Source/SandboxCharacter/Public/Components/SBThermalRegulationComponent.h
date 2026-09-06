#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBThermalTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBThermalRegulationComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBThermalComfortStateChanged, ESBThermalComfortState, OldState, ESBThermalComfortState, NewState, float, CoreTemperature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBHypothermiaTriggered, float, CoreTemperature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBHeatstrokeTriggered, float, CoreTemperature);

/**
 * Componente de termorregulação corporal, homeostase, hipotermia e choque térmico
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBThermalRegulationComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBThermalRegulationComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup APIs
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Thermal")
	void SetupThermalRegulation(float InitialCoreTemp = 37.0f, float InitialAmbientTemp = 22.0f);

	// Modifiers
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Thermal")
	void SetAmbientTemperature(float NewAmbientTemp);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Thermal")
	void SetThermalInsulation(float ColdInsulation, float HeatInsulation);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Thermal")
	void SetWetnessLevel(float InWetness);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Thermal")
	void SetNearbyHeatSource(float AddedHeatTemp);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Thermal")
	void SetWindChill(float InWindChill);

	// Simulation Tick
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Thermal")
	void SimulateThermalTick(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Thermal")
	float GetEffectiveAmbientTemperature() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Thermal")
	FSBThermalRegulationData GetThermalData() const { return ThermalData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Thermal")
	ESBThermalComfortState GetComfortState() const { return ThermalData.ComfortState; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Thermal")
	FSBThermalThresholdSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Thermal")
	FSBThermalComfortStateChanged OnThermalComfortStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Thermal")
	FSBHypothermiaTriggered OnHypothermiaTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Thermal")
	FSBHeatstrokeTriggered OnHeatstrokeTriggered;

private:
	void SyncTags();

	UPROPERTY()
	FSBThermalRegulationData ThermalData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
