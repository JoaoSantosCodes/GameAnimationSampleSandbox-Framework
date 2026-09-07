// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBThreadingTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBAsyncParallelDataComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBAsyncWorkFinished, float, ComputedValue);

/**
 * Componente com suporte a double-buffering thread-safe para cálculos assíncronos pesados
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCORE_API USBAsyncParallelDataComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBAsyncParallelDataComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Controls
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Threading")
	void RequestAsyncCalculation(float Multiplier, float Additive);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Threading")
	void CommitBackBuffer();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Threading")
	void SetFrontBufferValue(float Value);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Sandbox|Threading")
	float GetFrontBufferValue() const { return FrontBufferValue; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Threading")
	float GetBackBufferValue() const { return BackBufferValue; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Threading")
	bool IsAsyncWorkRunning() const { return bIsAsyncWorkRunning; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Threading")
	FSBAsyncWorkFinished OnAsyncWorkFinished;

private:
	void SyncTags();

	UPROPERTY()
	float FrontBufferValue;

	UPROPERTY()
	float BackBufferValue;

	UPROPERTY()
	bool bIsAsyncWorkRunning;

	UPROPERTY()
	TWeakObjectPtr<UActorComponent> CachedStateComp;
};
