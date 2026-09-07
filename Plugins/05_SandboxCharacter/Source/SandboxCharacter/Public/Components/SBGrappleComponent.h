// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBGrappleTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBGrappleComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnGrappleStateChanged, ESBGrappleState /*NewState*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnGrappleAnchored, const FVector& /*AnchorLocation*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnGrappleReleased, const FVector& /*ExitVelocity*/);

/**
 * Componente de gancho de fixação, tração rápida (winch pull), oscilação pendular e desacoplamento
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBGrappleComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBGrappleComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Grapple API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Grapple")
	bool AttachAnchorPoint(const FVector& InAnchorLocation, const FVector& InHitNormal);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Grapple")
	bool StartPull();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Grapple")
	bool StartSwing();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Grapple")
	bool ReleaseAnchorPoint(bool bApplyLaunchImpulse = true);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Grapple")
	void UpdateGrapplePhysics(float DeltaTime, const FVector& CurrentActorLocation);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Grapple")
	bool IsAttached() const { return AnchorData.bIsAttached; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Grapple")
	bool IsPulling() const { return AnchorData.bIsAttached && GrappleState == ESBGrappleState::Pulling; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Grapple")
	bool IsSwinging() const { return AnchorData.bIsAttached && GrappleState == ESBGrappleState::Swinging; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Grapple")
	ESBGrappleState GetGrappleState() const { return GrappleState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Grapple")
	FSBGrappleAnchorData GetAnchorData() const { return AnchorData; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Grapple")
	FSBGrappleSettings Settings;

	// Delegates
	FSBOnGrappleStateChanged OnGrappleStateChanged;
	FSBOnGrappleAnchored OnGrappleAnchored;
	FSBOnGrappleReleased OnGrappleReleased;

private:
	void SyncStateTags();

	UPROPERTY()
	ESBGrappleState GrappleState = ESBGrappleState::None;

	UPROPERTY()
	FSBGrappleAnchorData AnchorData;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
