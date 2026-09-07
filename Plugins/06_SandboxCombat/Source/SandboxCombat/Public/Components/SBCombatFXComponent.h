// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBCombatFXTypes.h"
#include "Engine/HitResult.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBCombatFXComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnWeaponTrailStateChanged, bool /*bIsActive*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSBOnImpactDecalSpawned, const FVector& /*Location*/, const FRotator& /*Rotation*/);

/**
 * Componente modular para controle de trilhas de corte de armas, partículas em sockets e decals de impacto físico
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBCombatFXComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBCombatFXComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Weapon Trail API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|FX")
	void ActivateWeaponTrail(const FSBWeaponTrailConfig& TrailConfig, USceneComponent* AttachComp = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|FX")
	void DeactivateWeaponTrail();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|FX")
	bool IsWeaponTrailActive() const { return ActiveTrailConfig.bIsActive; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|FX")
	FSBWeaponTrailConfig GetActiveTrailConfig() const { return ActiveTrailConfig; }

	// Impact Decals & Sockets API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|FX")
	bool SpawnImpactDecal(const FHitResult& HitResult, const FSBImpactDecalConfig& DecalConfig);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|FX")
	bool PlaySocketParticle(FName SocketName, USceneComponent* AttachComp = nullptr);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|FX")
	int32 GetSpawnedDecalCount() const { return SpawnedDecalHistory.Num(); }

	// Delegates
	FSBOnWeaponTrailStateChanged OnWeaponTrailStateChanged;
	FSBOnImpactDecalSpawned OnImpactDecalSpawned;

private:
	UPROPERTY()
	FSBWeaponTrailConfig ActiveTrailConfig;

	UPROPERTY()
	TArray<FSBCombatFXRequest> SpawnedDecalHistory;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
