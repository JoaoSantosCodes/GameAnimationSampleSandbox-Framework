#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "SBStateTreeCombatTasks.generated.h"

class AAIController;
class AActor;

/**
 * Dados de instância da tarefa de mover e atacar do StateTree
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMBAT_API FSBStateTreeTask_MoveAndAttackInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag ActionTag;

	UPROPERTY(VisibleAnywhere, Category = "State")
	bool bAttackTriggered = false;
};

/**
 * Tarefa do StateTree que persegue o alvo do combate e dispara a ação de ataque quando dentro do alcance
 */
USTRUCT(meta = (DisplayName = "SB Move and Attack Target", Category = "Sandbox|AI"))
struct SANDBOXCOMBAT_API FSBStateTreeTask_MoveAndAttack : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSBStateTreeTask_MoveAndAttackInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
