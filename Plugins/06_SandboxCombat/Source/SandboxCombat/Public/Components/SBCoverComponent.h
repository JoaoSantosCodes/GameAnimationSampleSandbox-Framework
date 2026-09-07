// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBCoverTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBCoverComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FSBOnCoverStateChanged, bool /*bInCover*/, ESBCoverType /*CoverType*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSBOnPeekStateChanged, bool /*bIsPeeking*/, ESBCoverEdge /*PeekEdge*/);

/**
 * Componente para ancoragem em cobertura física, alinhamento de postura e espionagem em quinas
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBCoverComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBCoverComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Cover API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Cover")
	bool EnterCover(const FSBCoverPoint& InCoverPoint);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Cover")
	void ExitCover();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Cover")
	bool IsInCover() const { return bIsInCover; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Cover")
	ESBCoverType GetCoverType() const { return CurrentCoverPoint.CoverType; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Cover")
	FSBCoverPoint GetCurrentCoverPoint() const { return CurrentCoverPoint; }

	// Peeking API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Cover")
	bool StartPeeking(ESBCoverEdge InEdge);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Cover")
	void StopPeeking();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Cover")
	bool IsPeeking() const { return bIsPeeking; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Cover")
	ESBCoverEdge GetCurrentPeekEdge() const { return CurrentPeekEdge; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Combat|Cover")
	FSBCoverSettings Settings;

	// Delegates
	FSBOnCoverStateChanged OnCoverStateChanged;
	FSBOnPeekStateChanged OnPeekStateChanged;

private:
	void SyncStateTags();

	UPROPERTY()
	FSBCoverPoint CurrentCoverPoint;

	UPROPERTY()
	bool bIsInCover = false;

	UPROPERTY()
	bool bIsPeeking = false;

	UPROPERTY()
	ESBCoverEdge CurrentPeekEdge = ESBCoverEdge::None;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
