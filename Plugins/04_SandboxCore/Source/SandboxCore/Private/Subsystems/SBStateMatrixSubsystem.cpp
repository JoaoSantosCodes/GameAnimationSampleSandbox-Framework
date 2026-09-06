#include "Subsystems/SBStateMatrixSubsystem.h"
#include "SBGameplayTags.h"

USBStateMatrixSubsystem::USBStateMatrixSubsystem()
{
}

void USBStateMatrixSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetSubsystem();
	RegisterStandardRules();
}

void USBStateMatrixSubsystem::Deinitialize()
{
	ResetSubsystem();
	Super::Deinitialize();
}

void USBStateMatrixSubsystem::RegisterExclusionRule(const FSBTagMutualExclusionRule& Rule)
{
	RegisteredRules.Add(Rule);
}

void USBStateMatrixSubsystem::RegisterStandardRules()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	// Gliding vs Swimming
	FGameplayTagContainer AquaticConflicts;
	AquaticConflicts.AddTag(Tags.State_Movement_Swimming);
	AquaticConflicts.AddTag(Tags.State_Movement_Swimming_Diving);
	RegisterExclusionRule(FSBTagMutualExclusionRule(Tags.State_Movement_Gliding, AquaticConflicts, ESBMatrixViolationAction::AutoResolvePrune));

	// Swimming vs Gliding
	FGameplayTagContainer GlidingConflicts;
	GlidingConflicts.AddTag(Tags.State_Movement_Gliding);
	GlidingConflicts.AddTag(Tags.State_Movement_Flying_Airborne);
	RegisterExclusionRule(FSBTagMutualExclusionRule(Tags.State_Movement_Swimming, GlidingConflicts, ESBMatrixViolationAction::AutoResolvePrune));

	// Dead vs Executing / Combat Active
	FGameplayTagContainer DeathConflicts;
	DeathConflicts.AddTag(Tags.State_Combat_Executing);
	DeathConflicts.AddTag(Tags.State_Combat_Attacking);
	RegisterExclusionRule(FSBTagMutualExclusionRule(Tags.State_Character_Dead, DeathConflicts, ESBMatrixViolationAction::RejectTransition));
}

bool USBStateMatrixSubsystem::ValidateTagAddition(AActor* TargetActor, const FGameplayTag& NewTag, const FGameplayTagContainer& CurrentTags, FGameplayTagContainer& OutPrunedTags)
{
	TotalEvaluations++;
	OutPrunedTags.Reset();

	for (const FSBTagMutualExclusionRule& Rule : RegisteredRules)
	{
		if (Rule.PrimaryTag == NewTag)
		{
			for (const FGameplayTag& Incompatible : Rule.IncompatibleTags)
			{
				if (CurrentTags.HasTag(Incompatible))
				{
					TotalViolationsDetected++;
					OnMatrixViolationDetected.Broadcast(TargetActor, NewTag, Incompatible);

					if (Rule.ViolationAction == ESBMatrixViolationAction::RejectTransition)
					{
						TotalTransitionsBlocked++;
						return false;
					}
					else if (Rule.ViolationAction == ESBMatrixViolationAction::AutoResolvePrune)
					{
						OutPrunedTags.AddTag(Incompatible);
						TotalTagsAutoPruned++;
					}
				}
			}
		}
	}

	return true;
}

FSBStateMatrixMetrics USBStateMatrixSubsystem::GetMetrics() const
{
	FSBStateMatrixMetrics Metrics;
	Metrics.TotalRulesRegistered = RegisteredRules.Num();
	Metrics.TotalEvaluations = TotalEvaluations;
	Metrics.TotalViolationsDetected = TotalViolationsDetected;
	Metrics.TotalTransitionsBlocked = TotalTransitionsBlocked;
	Metrics.TotalTagsAutoPruned = TotalTagsAutoPruned;
	return Metrics;
}

void USBStateMatrixSubsystem::ResetSubsystem()
{
	RegisteredRules.Empty();
	TotalEvaluations = 0;
	TotalViolationsDetected = 0;
	TotalTransitionsBlocked = 0;
	TotalTagsAutoPruned = 0;
}
