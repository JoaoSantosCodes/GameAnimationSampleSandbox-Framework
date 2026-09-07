// Copyright 2026 João Santos. All Rights Reserved.
#include "AI/StateTree/SBStateTreeCombatTasks.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBAbilityComponent.h"
#include "SBGameplayTags.h"

EStateTreeRunStatus FSBStateTreeTask_MoveAndAttack::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	if (!InstanceData.AIController)
	{
		InstanceData.AIController = Cast<AAIController>(Context.GetOwner());
		if (!InstanceData.AIController)
		{
			if (APawn* PawnOwner = Cast<APawn>(Context.GetOwner()))
			{
				InstanceData.AIController = Cast<AAIController>(PawnOwner->GetController());
			}
		}
	}

	if (!InstanceData.AIController || !InstanceData.TargetActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.ActionTag.IsValid())
	{
		InstanceData.ActionTag = FSBGameplayTags::Get().Combat_Action_Fire;
	}

	InstanceData.bAttackTriggered = false;

	APawn* ControlledPawn = InstanceData.AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), InstanceData.TargetActor->GetActorLocation());
	if (Distance <= InstanceData.AttackRange)
	{
		// Foco no alvo e ataca
		InstanceData.AIController->SetFocus(InstanceData.TargetActor);
		if (USBCombatComponent* CombatComp = ControlledPawn->FindComponentByClass<USBCombatComponent>())
		{
			CombatComp->RequestWeaponBehavior(InstanceData.ActionTag);
			InstanceData.bAttackTriggered = true;
			return EStateTreeRunStatus::Succeeded;
		}
		else if (USBAbilityComponent* AbilityComp = ControlledPawn->FindComponentByClass<USBAbilityComponent>())
		{
			AbilityComp->ActivateAbilityByTag(InstanceData.ActionTag);
			InstanceData.bAttackTriggered = true;
			return EStateTreeRunStatus::Succeeded;
		}
	}

	// Move até o alvo
	InstanceData.AIController->MoveToActor(InstanceData.TargetActor, InstanceData.AttackRange * 0.8f);
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSBStateTreeTask_MoveAndAttack::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!InstanceData.AIController || !InstanceData.TargetActor)
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* ControlledPawn = InstanceData.AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), InstanceData.TargetActor->GetActorLocation());
	if (Distance <= InstanceData.AttackRange)
	{
		InstanceData.AIController->StopMovement();
		InstanceData.AIController->SetFocus(InstanceData.TargetActor);

		if (USBCombatComponent* CombatComp = ControlledPawn->FindComponentByClass<USBCombatComponent>())
		{
			CombatComp->RequestWeaponBehavior(InstanceData.ActionTag);
			InstanceData.bAttackTriggered = true;
			return EStateTreeRunStatus::Succeeded;
		}
		else if (USBAbilityComponent* AbilityComp = ControlledPawn->FindComponentByClass<USBAbilityComponent>())
		{
			AbilityComp->ActivateAbilityByTag(InstanceData.ActionTag);
			InstanceData.bAttackTriggered = true;
			return EStateTreeRunStatus::Succeeded;
		}

		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FSBStateTreeTask_MoveAndAttack::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	if (InstanceData.AIController)
	{
		InstanceData.AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}
}
