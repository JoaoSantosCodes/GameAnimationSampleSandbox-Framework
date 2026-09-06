#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBMountTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBMountComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FSBOnMountStateChanged, ESBMountState /*NewState*/, AActor* /*Rider*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnMountGaitChanged, ESBMountGait /*NewGait*/);

/**
 * Componente de controle de montarias, fixação de cavaleiro e transições de andadura
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBMountComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBMountComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Mount API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Mount")
	bool Mount(AActor* InRider);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Mount")
	bool Dismount();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Mount")
	bool SetGait(ESBMountGait NewGait);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Mount")
	bool IsMounted() const { return RiderData.MountState == ESBMountState::Mounted; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Mount")
	ESBMountState GetMountState() const { return RiderData.MountState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Mount")
	AActor* GetRider() const { return RiderData.RiderActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Mount")
	ESBMountGait GetCurrentGait() const { return RiderData.CurrentGait; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Mount")
	float GetSpeedForGait(ESBMountGait Gait) const;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Mount")
	FSBMountSettings Settings;

	// Delegates
	FSBOnMountStateChanged OnMountStateChanged;
	FSBOnMountGaitChanged OnMountGaitChanged;

private:
	UPROPERTY()
	FSBMountRiderData RiderData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedMountStateComp;
};
