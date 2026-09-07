// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBCoreTestTypes.generated.h"

/**
 * Test double de estado para as suítes do SandboxCore.
 *
 * O SandboxCore é um módulo de Fundação e, pelo Princípio 7 (Zero Dependências
 * Circulares), não pode referenciar o USBStateComponent concreto do
 * 05_SandboxCharacter. Os componentes de Core resolvem o estado exclusivamente
 * via ISBStateComponentInterface, então as suítes fornecem esta implementação
 * mínima e local do contrato.
 */
UCLASS()
class USBCoreTestStateComponent : public UActorComponent, public ISBStateComponentInterface, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBCoreTestStateComponent()
	{
		PrimaryComponentTick.bCanEverTick = false;
	}

	// ISBComponentInterface — as suítes chamam Execute_OnInitialize/OnReady neste componente.
	// Sem implementar o contrato, Execute_ dispara assertion e derruba o processo de teste.
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override {}
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// ISBStateComponentInterface
	virtual bool HasTag_Implementation(FGameplayTag StateTag) const override { return ActiveStateTags.HasTagExact(StateTag); }
	virtual bool HasAny_Implementation(FGameplayTagContainer TagsContainer) const override { return ActiveStateTags.HasAny(TagsContainer); }
	virtual bool HasAll_Implementation(FGameplayTagContainer TagsContainer) const override { return ActiveStateTags.HasAll(TagsContainer); }
	virtual void AddTag_Implementation(FGameplayTag StateTag) override { if (StateTag.IsValid()) { ActiveStateTags.AddTag(StateTag); } }
	virtual void RemoveTag_Implementation(FGameplayTag StateTag) override { ActiveStateTags.RemoveTag(StateTag); }
	virtual FGameplayTagContainer GetActiveStateTags_Implementation() const override { return ActiveStateTags; }

	/** Acessores diretos para as asserções das specs (evitam Execute_ verboso nos testes). */
	bool HasTag(FGameplayTag StateTag) const { return HasTag_Implementation(StateTag); }
	void AddTag(FGameplayTag StateTag) { AddTag_Implementation(StateTag); }
	void RemoveTag(FGameplayTag StateTag) { RemoveTag_Implementation(StateTag); }

	const FGameplayTagContainer& GetActiveStateTags() const { return ActiveStateTags; }

private:
	UPROPERTY()
	FGameplayTagContainer ActiveStateTags;
};

/**
 * Listeners para os delegates dinâmicos do Core.
 *
 * DECLARE_DYNAMIC_MULTICAST_DELEGATE só aceita AddDynamic com UFUNCTION — lambdas
 * (AddLambda/AddWeakLambda) não são vinculáveis. Manter os delegates dinâmicos é
 * requisito do manifesto (BlueprintAssignable), então o teste fornece o receptor.
 */
UCLASS()
class USBCoreTestBudgetListener : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HandleBudgetExceeded(FName ComponentName, float ActualDurationUs, float BudgetThresholdUs)
	{
		bFired = true;
		CapturedName = ComponentName;
		CapturedDurationUs = ActualDurationUs;
		CapturedThresholdUs = BudgetThresholdUs;
	}

	bool bFired = false;
	FName CapturedName;
	float CapturedDurationUs = 0.0f;
	float CapturedThresholdUs = 0.0f;
};

UCLASS()
class USBCoreTestLockFreeListener : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void HandleEventProduced(int32 EventID, int32 EventType, float Value)
	{
		bFired = true;
		CapturedEventID = EventID;
		CapturedEventType = EventType;
		CapturedValue = Value;
	}

	bool bFired = false;
	int32 CapturedEventID = 0;
	int32 CapturedEventType = 0;
	float CapturedValue = 0.0f;
};
