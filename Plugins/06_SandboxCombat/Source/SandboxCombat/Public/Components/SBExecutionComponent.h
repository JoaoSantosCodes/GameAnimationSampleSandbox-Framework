#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBExecutionTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBExecutionComponent.generated.h"

class USBStateComponent;
class USBAttributeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnExecutionStarted, AActor*, Attacker, AActor*, Victim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnExecutionFinished, AActor*, Attacker, AActor*, Victim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnExecutionAborted, AActor*, Attacker, AActor*, Victim);

/**
 * Componente para sincronização e disparo de finalizações cinematográficas em dupla (Paired Sync Animations / Finishers)
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBExecutionComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBExecutionComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Execution")
	bool StartExecution(AActor* Victim, const FSBExecutionPairDefinition& Def);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Execution")
	void StopExecution(bool bAborted = false);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Execution")
	bool IsExecuting() const { return ActiveExecution.State == ESBExecutionState::Executing; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Execution")
	ESBExecutionState GetExecutionState() const { return ActiveExecution.State; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Execution")
	AActor* GetActiveVictim() const { return ActiveExecution.VictimActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Execution")
	AActor* GetActiveAttacker() const { return ActiveExecution.AttackerActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Execution")
	void CalculateAlignedVictimTransform(const FTransform& AttackerTransform, const FSBExecutionPairDefinition& Def, FTransform& OutVictimTransform) const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Execution")
	FSBOnExecutionStarted OnExecutionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Execution")
	FSBOnExecutionFinished OnExecutionFinished;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Execution")
	FSBOnExecutionAborted OnExecutionAborted;

private:
	UPROPERTY()
	FSBActiveExecution ActiveExecution;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedAttackerState;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedVictimState;

	UPROPERTY()
	TWeakObjectPtr<USBAttributeComponent> CachedVictimAttributes;
};
