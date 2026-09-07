// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBGameplayEffectTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBGameplayEffectComponent.generated.h"

class USBAttributeComponent;
class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnGameplayEffectApplied, const FSBGameplayEffectSpec&, Spec, int32, Stacks);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnGameplayEffectRemoved, const FSBGameplayEffectSpec&, Spec);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnGameplayEffectPeriodicTick, const FSBGameplayEffectSpec&, Spec, int32, Stacks);

/**
 * Componente responsável pelo gerenciamento de efeitos temporários/permanentes (Buffs, Debuffs, DoTs, HoTs)
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBGameplayEffectComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBGameplayEffectComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|GameplayEffects")
	bool ApplyGameplayEffectSpec(const FSBGameplayEffectSpec& Spec);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|GameplayEffects")
	bool RemoveGameplayEffectByTag(FGameplayTag EffectTag);

	UFUNCTION(BlueprintPure, Category = "Sandbox|GameplayEffects")
	bool HasActiveGameplayEffect(FGameplayTag EffectTag) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|GameplayEffects")
	int32 GetActiveEffectStacks(FGameplayTag EffectTag) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|GameplayEffects")
	const TArray<FSBActiveGameplayEffect>& GetActiveEffects() const { return ActiveEffects; }

	UFUNCTION(BlueprintCallable, Category = "Sandbox|GameplayEffects")
	void ClearAllEffects();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|GameplayEffects")
	FSBOnGameplayEffectApplied OnGameplayEffectApplied;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|GameplayEffects")
	FSBOnGameplayEffectRemoved OnGameplayEffectRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|GameplayEffects")
	FSBOnGameplayEffectPeriodicTick OnGameplayEffectPeriodicTick;

private:
	void ApplyModifiersToAttributes(const FSBGameplayEffectSpec& Spec, int32 OldStacks, int32 NewStacks);
	void RemoveModifiersFromAttributes(const FSBGameplayEffectSpec& Spec, int32 Stacks);
	void ApplyGrantedTags(const FSBGameplayEffectSpec& Spec);
	void RemoveGrantedTags(const FSBGameplayEffectSpec& Spec);

	UPROPERTY()
	TArray<FSBActiveGameplayEffect> ActiveEffects;

	UPROPERTY()
	TWeakObjectPtr<USBAttributeComponent> CachedAttributeComp;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
