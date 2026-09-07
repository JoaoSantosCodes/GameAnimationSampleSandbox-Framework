// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBExtractorTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBResourceExtractorComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBExtractorStateChanged, ESBExtractorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBExtractorItemHarvested, FName, ItemId, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBExtractorFluidHarvested, ESBFluidType, FluidType, float, Volume);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBExtractorTemperatureChanged, float, CurrentTemp, float, MaxSafeTemp);

/**
 * Componente de extração automatizada de recursos geológicos, brocas, poços de petróleo e bombas geotérmicas
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBResourceExtractorComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBResourceExtractorComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// Extractor API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Extractor")
	void SetupExtractor(ESBExtractorType InType, FName InItemId, ESBFluidType InFluidType, ESBResourceDepositPurity InPurity, float InBaseRate, float InPowerRequirement);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Extractor")
	void SetOverclockMultiplier(float InMultiplier);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Extractor")
	void SetPowerSupplied(bool bSupplied);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Extractor")
	void SetDepositDepleted(bool bDepleted);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Extractor")
	int32 WithdrawOutputItem(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Extractor")
	float WithdrawOutputFluid(ESBFluidType FluidType, float Volume);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Extractor")
	int32 GetOutputItemCount(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Extractor")
	float GetOutputFluidVolume(ESBFluidType FluidType) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Extractor")
	void SimulateExtractorTick(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Extractor")
	FSBExtractorData GetExtractorData() const { return ExtractorData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Extractor")
	ESBExtractorState GetExtractorState() const { return ExtractorData.ExtractorState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Extractor")
	bool IsExtracting() const { return ExtractorData.ExtractorState == ESBExtractorState::Extracting; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Extractor")
	FSBExtractorSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Extractor")
	FSBExtractorStateChanged OnExtractorStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Extractor")
	FSBExtractorItemHarvested OnExtractorItemHarvested;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Extractor")
	FSBExtractorFluidHarvested OnExtractorFluidHarvested;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Extractor")
	FSBExtractorTemperatureChanged OnExtractorTemperatureChanged;

private:
	void SyncExtractorState(ESBExtractorState NewState);
	void SyncTags();
	float GetPurityMultiplier() const;

	UPROPERTY()
	FSBExtractorData ExtractorData;

	TMap<FName, int32> OutputItemBuffer;
	TMap<ESBFluidType, float> OutputFluidBuffer;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
