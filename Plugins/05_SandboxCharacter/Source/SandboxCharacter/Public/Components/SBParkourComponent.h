// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBParkourTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBParkourComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FSBOnParkourActionStarted, ESBParkourActionType /*ActionType*/, const FSBParkourObstacleData& /*ObstacleData*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnParkourActionCompleted, ESBParkourActionType /*ActionType*/);

/**
 * Componente de detecção geométrica e execução de parkour, vaulting e mantling
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBParkourComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBParkourComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Parkour API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Parkour")
	FSBParkourObstacleData DetectObstacle(const FVector& WallLocation, const FVector& WallNormal, float ObstacleHeight, float ObstacleDepth);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Parkour")
	bool StartParkourAction(const FSBParkourObstacleData& InObstacleData);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Parkour")
	void CompleteParkourAction();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|Parkour")
	void CancelParkourAction();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Parkour")
	bool IsPerformingParkour() const { return bIsPerformingParkour; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Parkour")
	ESBParkourActionType GetCurrentParkourAction() const { return CurrentParkourAction; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|Parkour")
	FSBParkourObstacleData GetActiveObstacleData() const { return CurrentObstacleData; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|Parkour")
	FSBParkourSettings Settings;

	// Delegates
	FSBOnParkourActionStarted OnParkourActionStarted;
	FSBOnParkourActionCompleted OnParkourActionCompleted;

private:
	void SyncStateTags();

	UPROPERTY()
	FSBParkourObstacleData CurrentObstacleData;

	UPROPERTY()
	ESBParkourActionType CurrentParkourAction = ESBParkourActionType::None;

	UPROPERTY()
	bool bIsPerformingParkour = false;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
