// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBStateMatrixGuardComponent.h"
#include "Subsystems/SBStateMatrixSubsystem.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBStateMatrixGuardComponent::USBStateMatrixGuardComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBStateMatrixGuardComponent::OnInitialize_Implementation()
{
	SyncTags();
}

void USBStateMatrixGuardComponent::OnReady_Implementation()
{
	SyncTags();
}

void USBStateMatrixGuardComponent::OnShutdown_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Matrix_Verified);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Matrix_ConflictDetected);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Matrix_StrictEnforcement);
		}
	}
}

bool USBStateMatrixGuardComponent::TryApplyStateTag(FGameplayTag TagToApply)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	if (!StateComp)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		if (USBStateMatrixSubsystem* MatrixSubsystem = World->GetSubsystem<USBStateMatrixSubsystem>())
		{
			const FGameplayTagContainer CurrentTags = ISBStateComponentInterface::Execute_GetActiveStateTags(StateComp);
			FGameplayTagContainer PrunedTags;

			bool bValid = MatrixSubsystem->ValidateTagAddition(Owner, TagToApply, CurrentTags, PrunedTags);
			if (!bValid)
			{
				InterceptedConflictCount++;
				const FSBGameplayTags& Tags = FSBGameplayTags::Get();
				ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Matrix_ConflictDetected);
				return false;
			}

			// Remove tags incompatíveis podadas
			for (const FGameplayTag& Pruned : PrunedTags)
			{
				ISBStateComponentInterface::Execute_RemoveTag(StateComp, Pruned);
			}

			ISBStateComponentInterface::Execute_AddTag(StateComp, TagToApply);
			SyncTags();
			return true;
		}
	}

	ISBStateComponentInterface::Execute_AddTag(StateComp, TagToApply);
	SyncTags();
	return true;
}

void USBStateMatrixGuardComponent::SyncTags()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	if (!StateComp)
	{
		return;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Matrix_Verified);

	if (bStrictEnforcement)
	{
		ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Matrix_StrictEnforcement);
	}
	else
	{
		ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Matrix_StrictEnforcement);
	}
}
