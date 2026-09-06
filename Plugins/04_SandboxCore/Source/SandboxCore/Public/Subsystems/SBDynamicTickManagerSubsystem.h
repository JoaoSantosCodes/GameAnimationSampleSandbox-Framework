#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBTickThrottlingTypes.h"
#include "SBDynamicTickManagerSubsystem.generated.h"

class USBDynamicTickThrottlingComponent;

/**
 * Subsistema de orquestração e gerenciamento global de Tick Throttling e LOD de entidades
 */
UCLASS()
class SANDBOXCORE_API USBDynamicTickManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Optimization")
	void RegisterThrottledComponent(USBDynamicTickThrottlingComponent* Comp);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Optimization")
	void UnregisterThrottledComponent(USBDynamicTickThrottlingComponent* Comp);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Optimization")
	void UpdateAllLODs(const FVector& ViewerLocation);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Optimization")
	int32 GetRegisteredCount() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Optimization")
	int32 GetCountByLOD(ESBTickLODLevel LOD) const;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<USBDynamicTickThrottlingComponent>> RegisteredComponents;
};
