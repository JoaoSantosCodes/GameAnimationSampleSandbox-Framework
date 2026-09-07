// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBSwimTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBSwimComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnSwimStateChanged, ESBSwimState /*NewState*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSBOnOxygenChanged, float /*CurrentOxygen*/, float /*MaxOxygen*/);
DECLARE_MULTICAST_DELEGATE(FSBOnDrowningStarted);

/**
 * Componente de locomoção aquática, controle de oxigênio, mergulho e mecânica de afogamento
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBSwimComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBSwimComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Swimming API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Swim")
	void EnterWater(float InWaterSurfaceZ);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Swim")
	void ExitWater();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Swim")
	void StartDiving();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Swim")
	void SurfaceFromDive();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Swim")
	void UpdateWaterLocomotion(float DeltaTime, float CurrentZ);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Swim")
	void ConsumeOxygen(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Swim")
	void RecoverOxygen(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Swim")
	ESBSwimState GetSwimState() const { return SwimState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Swim")
	FSBOxygenData GetOxygenData() const { return OxygenData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Swim")
	bool IsSwimming() const { return SwimState != ESBSwimState::None; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Swim")
	bool IsDiving() const { return SwimState == ESBSwimState::Diving; }

	// Settings & Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Swim")
	FSBSwimSettings Settings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Swim")
	FSBOxygenData OxygenData;

	// Delegates
	FSBOnSwimStateChanged OnSwimStateChanged;
	FSBOnOxygenChanged OnOxygenChanged;
	FSBOnDrowningStarted OnDrowningStarted;

private:
	void SyncStateTags();

	UPROPERTY()
	ESBSwimState SwimState = ESBSwimState::None;

	UPROPERTY()
	float WaterSurfaceZ = 0.0f;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
