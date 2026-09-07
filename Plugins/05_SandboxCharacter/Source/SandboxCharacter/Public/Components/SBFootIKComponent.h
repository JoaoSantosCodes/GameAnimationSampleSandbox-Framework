// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBFootIKTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBFootIKComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnFootIKUpdated, const FSBFootIKResult& /*Result*/);

/**
 * Componente de ajuste de pés ao terreno, rotação de sola e compensação vertical da pelve
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBFootIKComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBFootIKComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Foot IK API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|FootIK")
	FSBFootIKResult CalculateFootIK(float LeftFootHeight, const FVector& LeftNormal, float RightFootHeight, const FVector& RightNormal);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|FootIK")
	FRotator CalculateRotationFromNormal(const FVector& HitNormal) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|FootIK")
	FSBFootIKResult GetFootIKResult() const { return CurrentResult; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Character|FootIK")
	bool IsIKEnabled() const { return bIsIKEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|FootIK")
	void SetIKEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Character|FootIK")
	void ResetFootIK();

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Character|FootIK")
	FSBFootIKSettings Settings;

	// Delegates
	FSBOnFootIKUpdated OnFootIKUpdated;

private:
	void SyncStateTags();

	UPROPERTY()
	FSBFootIKResult CurrentResult;

	UPROPERTY()
	bool bIsIKEnabled = true;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
