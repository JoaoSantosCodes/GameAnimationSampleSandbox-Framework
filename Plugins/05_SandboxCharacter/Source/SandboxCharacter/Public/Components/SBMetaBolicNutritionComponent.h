// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBNutritionTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBMetaBolicNutritionComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBHungerLevelChanged, ESBHungerLevel, OldLevel, ESBHungerLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBHydrationLevelChanged, ESBHydrationLevel, OldLevel, ESBHydrationLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBNutrientDeficiencyTriggered, FName, NutrientName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBStarvationTriggered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBCriticalDehydrationTriggered);

/**
 * Componente de nutrição metabólica, queima calórica, hidratação e deficiências vitamínicas
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBMetaBolicNutritionComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBMetaBolicNutritionComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Setup APIs
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Nutrition")
	void SetupMetabolicNutrition(float InitialCalories = 2000.0f, float InitialHydration = 100.0f);

	// Consumption
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Nutrition")
	void ConsumeFoodOrDrink(const FSBConsumableNutritionItem& Consumable);

	// Activity
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Nutrition")
	void SetActivityState(ESBMetabolicActivityState NewActivity);

	// Simulation
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Nutrition")
	void SimulateMetabolicTick(float DeltaTime, float AmbientHeatModifier = 1.0f);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Nutrition")
	FSBMetabolicNutritionData GetNutritionData() const { return NutritionData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Nutrition")
	ESBHungerLevel GetHungerLevel() const { return NutritionData.HungerLevel; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Nutrition")
	ESBHydrationLevel GetHydrationLevel() const { return NutritionData.HydrationLevel; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Nutrition")
	bool HasDeficiency(FName NutrientName) const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Nutrition")
	FSBHungerLevelChanged OnHungerLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Nutrition")
	FSBHydrationLevelChanged OnHydrationLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Nutrition")
	FSBNutrientDeficiencyTriggered OnNutrientDeficiencyTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Nutrition")
	FSBStarvationTriggered OnStarvationTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Nutrition")
	FSBCriticalDehydrationTriggered OnCriticalDehydrationTriggered;

private:
	void SyncTags();
	float GetActivityMultiplier() const;

	UPROPERTY()
	FSBMetabolicNutritionData NutritionData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
