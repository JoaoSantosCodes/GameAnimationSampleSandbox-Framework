// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBMotionWarpTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBMotionWarpComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBOnMotionWarpStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBOnMotionWarpCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBOnMotionWarpAborted);

/**
 * Componente de aproximação dinâmica e ajuste postural em combate corpo a corpo (Motion Warping / Target Snapping)
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBMotionWarpComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBMotionWarpComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|MotionWarp")
	void StartMotionWarp(const FSBMotionWarpTarget& Target, const FSBMotionWarpConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|MotionWarp")
	void StartMotionWarpToActor(AActor* TargetActor, float Duration = 0.3f, float OffsetDistance = 120.0f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|MotionWarp")
	void StopMotionWarp(bool bAborted = false);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|MotionWarp")
	bool IsWarping() const { return WarpState == ESBMotionWarpState::Warping; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|MotionWarp")
	ESBMotionWarpState GetWarpState() const { return WarpState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|MotionWarp")
	void CalculateWarpTransform(float Alpha, FVector& OutLocation, FRotator& OutRotation) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|MotionWarp")
	FVector GetCalculatedTargetLocation() const { return CalculatedTargetLocation; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|MotionWarp")
	FRotator GetCalculatedTargetRotation() const { return CalculatedTargetRotation; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|MotionWarp")
	FSBOnMotionWarpStarted OnMotionWarpStarted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|MotionWarp")
	FSBOnMotionWarpCompleted OnMotionWarpCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|MotionWarp")
	FSBOnMotionWarpAborted OnMotionWarpAborted;

private:
	UPROPERTY()
	FSBMotionWarpTarget CurrentTarget;

	UPROPERTY()
	FSBMotionWarpConfig CurrentConfig;

	ESBMotionWarpState WarpState = ESBMotionWarpState::Inactive;
	float ElapsedTime = 0.0f;

	FVector StartLocation = FVector::ZeroVector;
	FRotator StartRotation = FRotator::ZeroRotator;
	FVector CalculatedTargetLocation = FVector::ZeroVector;
	FRotator CalculatedTargetRotation = FRotator::ZeroRotator;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
