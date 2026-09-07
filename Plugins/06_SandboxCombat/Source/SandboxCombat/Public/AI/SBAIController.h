// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "SmartObjectTypes.h"
#include "SBAIController.generated.h"

class USBCombatComponent;
class USBStateComponent;
class USBAttributeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBBossPhaseChangedSignature, int32, NewPhase);

UCLASS(Blueprintable, BlueprintType)
class SANDBOXCOMBAT_API ASBAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASBAIController();

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|AI")
	FSBBossPhaseChangedSignature OnBossPhaseChanged;

	/** Thresholds de porcentagem de vida do boss para mudar de fase (ex: 0.75f, 0.50f, 0.25f) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|AI|Boss")
	TArray<float> BossPhaseHPThresholds;

	UFUNCTION(BlueprintPure, Category = "Sandbox|AI|Boss")
	int32 GetCurrentBossPhase() const { return CurrentBossPhase; }

	// Busca objetos inteligentes próximos baseados no filtro de tags de atividade
	UFUNCTION(BlueprintCallable, Category = "Sandbox|AI|SmartObjects")
	bool FindNearbySmartObjects(TArray<FSmartObjectRequestResult>& OutResults, FGameplayTagQuery ActivityFilter, float SearchRadius = 1000.0f) const;

	// Reivindica um slot de Smart Object
	UFUNCTION(BlueprintCallable, Category = "Sandbox|AI|SmartObjects")
	bool ClaimSmartObjectSlot(const FSmartObjectRequestResult& RequestResult, FSmartObjectClaimHandle& OutClaimHandle);

	// Libera a reivindicação de um slot
	UFUNCTION(BlueprintCallable, Category = "Sandbox|AI|SmartObjects")
	bool ReleaseSmartObjectSlot(const FSmartObjectClaimHandle& ClaimHandle);

	// Obtém a localização e transformação de um slot reivindicado
	UFUNCTION(BlueprintPure, Category = "Sandbox|AI|SmartObjects")
	bool GetSmartObjectSlotTransform(const FSmartObjectClaimHandle& ClaimHandle, FTransform& OutSlotTransform) const;

	UFUNCTION()
	void HandleAgroTargetChanged(APawn* NewTarget);

	UFUNCTION()
	void HandleStateChanged(FGameplayTag StateTag, bool bAdded);

	UFUNCTION()
	void HandleAttributeChanged(FGameplayTag AttributeTag, float NewValue, float OldValue, AActor* InInstigator);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Sandbox|AI")
	TWeakObjectPtr<USBCombatComponent> CachedCombatComp;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Sandbox|AI")
	TWeakObjectPtr<USBStateComponent> CachedStateComp;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Sandbox|AI")
	TWeakObjectPtr<USBAttributeComponent> CachedAttrComp;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Sandbox|AI|Boss")
	int32 CurrentBossPhase = 0;

private:
	void SetupBossPhaseHPTracking();
};
