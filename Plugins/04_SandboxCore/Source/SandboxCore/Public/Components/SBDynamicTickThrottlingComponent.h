#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBTickThrottlingTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBDynamicTickThrottlingComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBLODLevelChanged, ESBTickLODLevel, NewLOD);

/**
 * Componente de otimização de LOD e Tick Throttling dinâmico para simulações massivas
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCORE_API USBDynamicTickThrottlingComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBDynamicTickThrottlingComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Configuration & Controls
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Optimization")
	void SetupThrottling(const FSBTickThrottlingSettings& InSettings);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Optimization")
	void UpdateDistanceToViewer(float Distance, bool bInFrustum = true);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Optimization")
	bool AdvanceTick(float DeltaTime, float& OutConsolidatedDeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Optimization")
	void ForceLOD(ESBTickLODLevel NewLOD);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Optimization")
	FSBTickThrottlingState GetThrottlingState() const { return ThrottlingState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Optimization")
	ESBTickLODLevel GetCurrentLOD() const { return ThrottlingState.CurrentLOD; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Optimization")
	FSBTickThrottlingSettings GetSettings() const { return Settings; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Optimization")
	FSBLODLevelChanged OnLODLevelChanged;

private:
	void SyncTags();
	void RecalculateLOD();

	UPROPERTY(EditAnywhere, Category = "Throttling")
	FSBTickThrottlingSettings Settings;

	UPROPERTY()
	FSBTickThrottlingState ThrottlingState;

	UPROPERTY()
	bool bIsFrustumVisible;

	UPROPERTY()
	TWeakObjectPtr<UActorComponent> CachedStateComp;
};
