#include "Components/SBExecutionComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBAttributeComponent.h"
#include "SBGameplayTags.h"

USBExecutionComponent::USBExecutionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBExecutionComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedAttackerState = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBExecutionComponent::OnShutdown_Implementation()
{
	StopExecution(true);
}

void USBExecutionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActiveExecution.State == ESBExecutionState::Executing)
	{
		AActor* Attacker = ActiveExecution.AttackerActor.Get();
		AActor* Victim = ActiveExecution.VictimActor.Get();

		if (!IsValid(Attacker) || !IsValid(Victim))
		{
			StopExecution(true);
			return;
		}

		ActiveExecution.ElapsedTime += DeltaTime;
		if (ActiveExecution.ElapsedTime >= ActiveExecution.Definition.ExecutionDuration)
		{
			StopExecution(false);
		}
	}
}

bool USBExecutionComponent::StartExecution(AActor* Victim, const FSBExecutionPairDefinition& Def)
{
	AActor* Attacker = GetOwner();
	if (!IsValid(Attacker) || !IsValid(Victim) || IsExecuting())
	{
		return false;
	}

	if (!CachedAttackerState.IsValid())
	{
		CachedAttackerState = Attacker->FindComponentByClass<USBStateComponent>();
	}

	CachedVictimState = Victim->FindComponentByClass<USBStateComponent>();
	CachedVictimAttributes = Victim->FindComponentByClass<USBAttributeComponent>();

	ActiveExecution.AttackerActor = Attacker;
	ActiveExecution.VictimActor = Victim;
	ActiveExecution.Definition = Def;
	ActiveExecution.State = ESBExecutionState::Executing;
	ActiveExecution.ElapsedTime = 0.0f;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (CachedAttackerState.IsValid())
	{
		CachedAttackerState->AddTag(Tags.State_Combat_Executing);
		if (Def.bGrantInvulnerabilityToAttacker)
		{
			CachedAttackerState->AddTag(Tags.State_Combat_Invulnerable);
		}
	}

	if (CachedVictimState.IsValid())
	{
		CachedVictimState->AddTag(Tags.State_Combat_Executed);
	}

	// Alinha a vítima espacialmente em relação ao atacante
	FTransform AlignedVictimTransform;
	CalculateAlignedVictimTransform(Attacker->GetActorTransform(), Def, AlignedVictimTransform);
	Victim->SetActorLocationAndRotation(AlignedVictimTransform.GetLocation(), AlignedVictimTransform.GetRotation().Rotator());

	OnExecutionStarted.Broadcast(Attacker, Victim);
	return true;
}

void USBExecutionComponent::StopExecution(bool bAborted)
{
	if (ActiveExecution.State != ESBExecutionState::Executing)
	{
		return;
	}

	AActor* Attacker = ActiveExecution.AttackerActor.Get();
	AActor* Victim = ActiveExecution.VictimActor.Get();

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (CachedAttackerState.IsValid())
	{
		CachedAttackerState->RemoveTag(Tags.State_Combat_Executing);
		CachedAttackerState->RemoveTag(Tags.State_Combat_Invulnerable);
	}

	if (CachedVictimState.IsValid())
	{
		CachedVictimState->RemoveTag(Tags.State_Combat_Executed);
	}

	if (!bAborted && IsValid(Victim))
	{
		if (CachedVictimAttributes.IsValid())
		{
			float CurrentHealth = CachedVictimAttributes->GetAttributeValue(Tags.Attribute_Health);
			CachedVictimAttributes->SetAttributeBaseValue(Tags.Attribute_Health, FMath::Max(0.0f, CurrentHealth - ActiveExecution.Definition.DamageOnFinish));
		}
	}

	ActiveExecution.State = bAborted ? ESBExecutionState::Aborted : ESBExecutionState::Finished;

	if (bAborted)
	{
		OnExecutionAborted.Broadcast(Attacker, Victim);
	}
	else
	{
		OnExecutionFinished.Broadcast(Attacker, Victim);
	}
}

void USBExecutionComponent::CalculateAlignedVictimTransform(const FTransform& AttackerTransform, const FSBExecutionPairDefinition& Def, FTransform& OutVictimTransform) const
{
	FVector WorldVictimLocation = AttackerTransform.TransformPosition(Def.RelativeVictimLocation);
	FRotator WorldVictimRotation = (AttackerTransform.GetRotation() * Def.RelativeVictimRotation.Quaternion()).Rotator();
	OutVictimTransform = FTransform(WorldVictimRotation, WorldVictimLocation, AttackerTransform.GetScale3D());
}
