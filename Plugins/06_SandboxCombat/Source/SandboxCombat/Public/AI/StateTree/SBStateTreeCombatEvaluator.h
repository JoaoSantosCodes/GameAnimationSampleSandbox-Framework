#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "GameplayTagContainer.h"
#include "SBStateTreeCombatEvaluator.generated.h"

class AAIController;
class ASBCharacter;
class USBCombatComponent;
class USBStateComponent;
class USBAttributeComponent;

/**
 * Dados de instância do Evaluator de Combate do StateTree
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMBAT_API FSBStateTreeCombatEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Output")
	float HealthRatio = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Output")
	bool bIsStunned = false;

	UPROPERTY(VisibleAnywhere, Category = "Output")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, Category = "Output")
	TObjectPtr<AActor> TargetActor = nullptr;
};

/**
 * Evaluator do StateTree para extrair e atualizar dados de combate e atributos da IA continuamente
 */
USTRUCT(meta = (DisplayName = "SB Combat Evaluator", Category = "Sandbox|AI"))
struct SANDBOXCOMBAT_API FSBStateTreeCombatEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSBStateTreeCombatEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
