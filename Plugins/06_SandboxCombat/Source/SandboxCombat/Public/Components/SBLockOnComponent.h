// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBLockOnTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBLockOnComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnLockOnTargetChanged, AActor*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBOnLockOnTargetLost);

/**
 * Componente responsável pelo sistema de mira, foco e trava em alvos (Lock-On Target)
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBLockOnComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBLockOnComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|LockOn")
	bool ToggleLockOn();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|LockOn")
	bool LockOnTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|LockOn")
	void UnlockTarget();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|LockOn")
	bool SwitchTarget(ESBLockOnSwitchDirection Direction);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|LockOn")
	AActor* FindBestTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|LockOn")
	void FindCandidates(TArray<FSBLockOnCandidate>& OutCandidates) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|LockOn")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|LockOn")
	bool IsLockedOn() const { return CurrentTarget.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|LockOn")
	FRotator GetDesiredRotationToTarget() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Combat|LockOn")
	FSBLockOnSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|LockOn")
	FSBOnLockOnTargetChanged OnLockOnTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|LockOn")
	FSBOnLockOnTargetLost OnLockOnTargetLost;

private:
	void SetTargetStateTag(AActor* Target, bool bIsTarget);

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTarget;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
