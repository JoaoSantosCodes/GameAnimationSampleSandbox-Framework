#include "AI/StateTree/SBStateTreeCombatEvaluator.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBAttributeComponent.h"
#include "SBGameplayTags.h"

void FSBStateTreeCombatEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
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

	Tick(Context, 0.0f);
}

void FSBStateTreeCombatEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	AAIController* AIC = InstanceData.AIController;
	if (!AIC)
	{
		AIC = Cast<AAIController>(Context.GetOwner());
		if (!AIC)
		{
			if (APawn* PawnOwner = Cast<APawn>(Context.GetOwner()))
			{
				AIC = Cast<AAIController>(PawnOwner->GetController());
			}
		}
		InstanceData.AIController = AIC;
	}

	if (!AIC)
	{
		return;
	}

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn)
	{
		InstanceData.bIsDead = true;
		InstanceData.HealthRatio = 0.0f;
		InstanceData.TargetActor = nullptr;
		return;
	}

	// 1. Consulta Atributos (Health)
	if (USBAttributeComponent* AttrComp = Pawn->FindComponentByClass<USBAttributeComponent>())
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();
		float Health = AttrComp->GetAttributeValue(Tags.Attribute_Health);
		float MaxHealth = AttrComp->GetAttributeValue(Tags.Attribute_MaxHealth);

		if (MaxHealth > 0.0f)
		{
			InstanceData.HealthRatio = FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f);
		}
		else
		{
			InstanceData.HealthRatio = 0.0f;
		}

		InstanceData.bIsDead = (Health <= 0.0f);
	}

	// 2. Consulta Estados (Tags)
	if (USBStateComponent* StateComp = Pawn->FindComponentByClass<USBStateComponent>())
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();
		InstanceData.bIsStunned = StateComp->HasTag(Tags.State_Character_Stunned) || StateComp->HasTag(Tags.State_Character_Frozen);
		if (StateComp->HasTag(Tags.State_Character_Dead))
		{
			InstanceData.bIsDead = true;
		}
	}

	// 3. Consulta Agro Target
	if (USBCombatComponent* CombatComp = Pawn->FindComponentByClass<USBCombatComponent>())
	{
		InstanceData.TargetActor = CombatComp->GetHighestAgroTarget();
	}
}
