// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBZiplineTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBZiplineComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnZiplineStateChanged, ESBZiplineState /*NewState*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnZiplineProgress, float /*AlphaProgress*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnZiplineDismounted, const FVector& /*ExitVelocity*/);

/**
 * Componente de tirolesa, cabos de deslizamento e travessia suspensa
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBZiplineComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBZiplineComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Zipline API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Zipline")
	bool AttachToZipline(const FVector& InStartPoint, const FVector& InEndPoint);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Zipline")
	bool DetachFromZipline(bool bApplyLaunchImpulse = true);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Zipline")
	void UpdateZiplineTravel(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Zipline")
	bool IsRiding() const { return RideData.bIsRiding; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Zipline")
	ESBZiplineState GetZiplineState() const { return ZiplineState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Zipline")
	FSBZiplineRideData GetRideData() const { return RideData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Zipline")
	FVector GetCalculatedLocation() const;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Zipline")
	FSBZiplineSettings Settings;

	// Delegates
	FSBOnZiplineStateChanged OnZiplineStateChanged;
	FSBOnZiplineProgress OnZiplineProgress;
	FSBOnZiplineDismounted OnZiplineDismounted;

private:
	void SyncStateTags();

	UPROPERTY()
	ESBZiplineState ZiplineState = ESBZiplineState::None;

	UPROPERTY()
	FSBZiplineRideData RideData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
