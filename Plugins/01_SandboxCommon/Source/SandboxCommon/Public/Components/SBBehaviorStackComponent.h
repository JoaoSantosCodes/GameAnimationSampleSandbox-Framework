#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBCommonTypes.h"
#include "Behaviors/SBGameplayBehavior.h"
#include "Interfaces/SBDebugInterface.h"
#include "SBBehaviorStackComponent.generated.h"

class USBGameplayBehaviorDefinition;

UCLASS(BlueprintType)
class SANDBOXCOMMON_API USBBehaviorStackComponent : public UGameFrameworkComponent, public ISBComponentInterface, public ISBDebugInterface
{
	GENERATED_BODY()

public:
	USBBehaviorStackComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override {}
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// API Pública da Pilha
	UFUNCTION(BlueprintCallable, Category = "Sandbox|BehaviorStack")
	virtual bool RequestBehavior(FGameplayTag BehaviorTag);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|BehaviorStack")
	virtual void StopBehavior(FGameplayTag BehaviorTag, bool bSkipServerNotify = false, bool bSkipClientNotify = false);

	UFUNCTION(BlueprintPure, Category = "Sandbox|BehaviorStack")
	virtual bool HasBehavior(FGameplayTag BehaviorTag) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|BehaviorStack")
	USBGameplayBehavior* GetCurrentBehavior() const;

	// Helpers de Busca
	UFUNCTION(BlueprintPure, Category = "Sandbox|BehaviorStack")
	USBGameplayBehavior* FindAvailableBehaviorByTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|BehaviorStack")
	USBGameplayBehavior* FindActiveBehaviorByTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|BehaviorStack")
	void AddAvailableBehavior(USBGameplayBehavior* Behavior) { if (Behavior) AvailableBehaviors.AddUnique(Behavior); }

	// Helpers de C++ baseados em templates para evitar Cast<> dispersos
	template<typename T>
	T* GetActiveBehaviorOfType() const
	{
		for (USBGameplayBehavior* Behavior : ActiveBehaviors)
		{
			if (T* Casted = Cast<T>(Behavior))
			{
				return Casted;
			}
		}
		return nullptr;
	}

	template<typename T>
	TArray<T*> GetActiveBehaviorsOfType() const
	{
		TArray<T*> Results;
		for (USBGameplayBehavior* Behavior : ActiveBehaviors)
		{
			if (T* Casted = Cast<T>(Behavior))
			{
				Results.Add(Casted);
			}
		}
		return Results;
	}

	// ISBDebugInterface
	virtual void GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const override;

	// Controle de Mutação da Pilha (RAII/Reentrância)
	void IncrementMutationDepth();
	void DecrementMutationDepth();

protected:
	// Gancho Virtual para notificação de rede por domínio
	virtual void OnBehaviorEjected(FGameplayTag BehaviorTag, bool bSkipServerNotify, bool bSkipClientNotify) {}

	// Pilha ativa (ordenada por prioridade decrescente)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Sandbox|BehaviorStack")
	TArray<TObjectPtr<USBGameplayBehavior>> ActiveBehaviors;

	// Todos os comportamentos instanciados
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Sandbox|BehaviorStack")
	TArray<TObjectPtr<USBGameplayBehavior>> AvailableBehaviors;

	// Fila de comportamentos aguardando remoção
	UPROPERTY(Transient)
	TArray<TObjectPtr<USBGameplayBehavior>> DeferredExits;

	// Fila de comportamentos aguardando ativação
	UPROPERTY(Transient)
	TArray<TObjectPtr<USBGameplayBehavior>> DeferredEntries;

	int32 StackMutationDepth = 0;
	bool bIsResolvingDeferred = false;

	void ResolveDeferredExits();
	void ResolveDeferredEntries();
	void SortActiveStack();
	FSBBehaviorContext BuildBehaviorContext(float DeltaTime, FSBGameplayContext& OutGameplayCtx, FSBFrameworkContext& OutFrameworkCtx) const;
};

// RAII Guard para mutações de pilha seguras
struct SANDBOXCOMMON_API FSBBehaviorStackMutationGuard
{
	USBBehaviorStackComponent* OwnerStack;

	FSBBehaviorStackMutationGuard(USBBehaviorStackComponent* InOwner)
		: OwnerStack(InOwner)
	{
		if (OwnerStack)
		{
			OwnerStack->IncrementMutationDepth();
		}
	}

	~FSBBehaviorStackMutationGuard()
	{
		if (OwnerStack)
		{
			OwnerStack->DecrementMutationDepth();
		}
	}
};

UCLASS()
class SANDBOXCOMMON_API USBMockGameplayBehavior : public USBGameplayBehavior
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bEntered = false;

	UPROPERTY()
	bool bExited = false;

	UPROPERTY()
	FGameplayTag TagToRequestOnExit;

	UPROPERTY()
	TObjectPtr<USBBehaviorStackComponent> StackToRequestOnExit = nullptr;

	virtual void Enter_Implementation(const FSBBehaviorContext& Context) override;
	virtual void Exit_Implementation(const FSBBehaviorContext& Context) override;
};
