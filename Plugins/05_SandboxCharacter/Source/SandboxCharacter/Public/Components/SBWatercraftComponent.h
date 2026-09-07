// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBWatercraftTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBWatercraftComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnWatercraftStateChanged, ESBWatercraftState /*NewState*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnWatercraftAnchorChanged, bool /*bIsAnchored*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnWatercraftPilotChanged, AActor* /*Pilot*/);

/**
 * Componente de controle de embarcações aquáticas, barcos a motor, leme e ancoragem
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBWatercraftComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBWatercraftComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Watercraft API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Watercraft")
	bool EnterWatercraft(AActor* InPilot);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Watercraft")
	bool ExitWatercraft(AActor* InPilot);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Watercraft")
	void SetWaterSurfaceLevel(float InWaterZ);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Watercraft")
	bool DropAnchor();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Watercraft")
	bool RaiseAnchor();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Watercraft")
	void SetThrottleInput(float InThrottle);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Watercraft")
	void SetRudderInput(float InRudder);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Watercraft")
	void UpdateWatercraftPhysics(float DeltaTime, const FVector& CurrentActorLocation);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Watercraft")
	bool IsPilot(const AActor* InPilot) const { return CurrentPilot.IsValid() && CurrentPilot.Get() == InPilot; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Watercraft")
	AActor* GetPilot() const { return CurrentPilot.Get(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Watercraft")
	bool IsAnchored() const { return NavigationData.bIsAnchored; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Watercraft")
	FSBWatercraftNavigationData GetNavigationData() const { return NavigationData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Watercraft")
	ESBWatercraftState GetWatercraftState() const { return WatercraftState; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Watercraft")
	FSBWatercraftSettings Settings;

	// Delegates
	FSBOnWatercraftStateChanged OnWatercraftStateChanged;
	FSBOnWatercraftAnchorChanged OnWatercraftAnchorChanged;
	FSBOnWatercraftPilotChanged OnWatercraftPilotChanged;

private:
	void SyncSailingTags();

	UPROPERTY()
	ESBWatercraftState WatercraftState = ESBWatercraftState::Docked;

	UPROPERTY()
	FSBWatercraftNavigationData NavigationData;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentPilot;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
