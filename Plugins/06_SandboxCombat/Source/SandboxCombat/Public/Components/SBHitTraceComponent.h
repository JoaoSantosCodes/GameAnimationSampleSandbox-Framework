#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBHitTraceTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBHitTraceComponent.generated.h"

class USBStateComponent;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSBOnMeleeHit, AActor*, HitActor, const FHitResult&, HitResult, float, Damage, FGameplayTag, AttackTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnHitTraceStarted, FGameplayTag, AttackTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnHitTraceEnded, int32, TotalHitsRecorded);

/**
 * Componente de detecção de colisão e impactos de ataque corpo a corpo multi-socket anti-tunneling
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBHitTraceComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBHitTraceComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|HitTrace")
	void StartHitTrace(const FSBHitTraceSettings& Settings, USceneComponent* InSourceComponent = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|HitTrace")
	void StopHitTrace();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|HitTrace")
	void SetSourceComponent(USceneComponent* InSourceComponent);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|HitTrace")
	bool IsTracingActive() const { return bIsTracingActive; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|HitTrace")
	int32 GetHitActorsCount() const { return HitActorsInCurrentSwing.Num(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|HitTrace")
	const FSBHitTraceSettings& GetActiveSettings() const { return ActiveSettings; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|HitTrace")
	FSBOnMeleeHit OnMeleeHit;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|HitTrace")
	FSBOnHitTraceStarted OnHitTraceStarted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|HitTrace")
	FSBOnHitTraceEnded OnHitTraceEnded;

private:
	void PerformTraceStep();

	UPROPERTY()
	FSBHitTraceSettings ActiveSettings;

	bool bIsTracingActive = false;

	UPROPERTY()
	TMap<FName, FVector> PreviousSocketLocations;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> HitActorsInCurrentSwing;

	UPROPERTY()
	TWeakObjectPtr<USceneComponent> SourceComponent;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
