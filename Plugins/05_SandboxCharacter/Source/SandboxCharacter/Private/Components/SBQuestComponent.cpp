// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBQuestComponent.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "SBGameplayTags.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBExperienceComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"

USBQuestComponent::USBQuestComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USBQuestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBQuestComponent, ActiveQuests);
}

void USBQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1. Assina no barramento de eventos localmente
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	USBEventSubsystem* EventSubsystem = GI ? GI->GetSubsystem<USBEventSubsystem>() : nullptr;

	if (EventSubsystem)
	{
		// Escuta quando itens são adicionados ao inventário
		ItemAddedHandle = EventSubsystem->SubscribeToEventNative(
			FSBGameplayTags::Get().Event_Inventory_ItemAdded,
			ESBEventPriority::Medium,
			FSBNativeEventDelegate::CreateUObject(this, &USBQuestComponent::HandleItemAdded)
		);

		// Escuta quando passos físicos ocorrem
		FootstepHandle = EventSubsystem->SubscribeToEventNative(
			FSBGameplayTags::Get().Event_Character_Footstep,
			ESBEventPriority::Medium,
			FSBNativeEventDelegate::CreateUObject(this, &USBQuestComponent::HandleFootstep)
		);
	}
}

void USBQuestComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	USBEventSubsystem* EventSubsystem = GI ? GI->GetSubsystem<USBEventSubsystem>() : nullptr;

	if (EventSubsystem)
	{
		if (ItemAddedHandle.IsValid())
		{
			EventSubsystem->UnsubscribeFromEventNative(FSBGameplayTags::Get().Event_Inventory_ItemAdded, ItemAddedHandle);
		}
		if (FootstepHandle.IsValid())
		{
			EventSubsystem->UnsubscribeFromEventNative(FSBGameplayTags::Get().Event_Character_Footstep, FootstepHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void USBQuestComponent::OnRep_ActiveQuests()
{
	// Notifica listeners no cliente sobre as atualizações das quests
	for (const FSBActiveQuest& ActiveQuest : ActiveQuests)
	{
		if (ActiveQuest.bIsCompleted)
		{
			OnQuestCompleted.Broadcast(ActiveQuest.QuestData);
		}
	}
}

void USBQuestComponent::HandleItemAdded(FGameplayTag EventTag, UObject* Payload)
{
	USBInventoryEventPayload* InvPayload = Cast<USBInventoryEventPayload>(Payload);
	if (!InvPayload || InvPayload->TargetPawn != GetOwner()) return;

	UObject* ItemInstance = InvPayload->ItemInstance;
	if (!ItemInstance) return;

	// Lê as propriedades por reflexão para evitar acoplamento direto com a extensão de inventário
	UObject* ItemDef = nullptr;
	int32 StackCount = 1;

	if (FObjectProperty* ItemDefProp = CastField<FObjectProperty>(ItemInstance->GetClass()->FindPropertyByName(TEXT("ItemDef"))))
	{
		ItemDef = ItemDefProp->GetObjectPropertyValue_InContainer(ItemInstance);
	}

	if (FIntProperty* StackCountProp = CastField<FIntProperty>(ItemInstance->GetClass()->FindPropertyByName(TEXT("StackCount"))))
	{
		StackCount = StackCountProp->GetPropertyValue_InContainer(ItemInstance);
	}

	if (ItemDef)
	{
		// Obtém a GameplayTagContainer do ItemDef por reflexão
		FGameplayTagContainer ItemTags;
		if (FStructProperty* TagsProp = CastField<FStructProperty>(ItemDef->GetClass()->FindPropertyByName(TEXT("ItemTags"))))
		{
			if (const FGameplayTagContainer* TagsPtr = TagsProp->ContainerPtrToValuePtr<FGameplayTagContainer>(ItemDef))
			{
				ItemTags = *TagsPtr;
			}
		}

		// Percorre os objetivos e progredi os correspondentes
		for (const FSBActiveQuest& ActiveQuest : ActiveQuests)
		{
			if (ActiveQuest.bIsCompleted) continue;

			for (const FSBQuestObjective& Objective : ActiveQuest.QuestData->Objectives)
			{
				if (Objective.ObjectiveTag.IsValid() && (ItemTags.HasTag(Objective.ObjectiveTag) || Objective.ObjectiveTag == ItemTags.GetByIndex(0)))
				{
					ProgressObjective(Objective.ObjectiveTag, StackCount);
				}
			}
		}
	}
}

void USBQuestComponent::HandleFootstep(FGameplayTag EventTag, UObject* Payload)
{
	USBFootstepEventPayload* FootPayload = Cast<USBFootstepEventPayload>(Payload);
	if (!FootPayload || FootPayload->TargetPawn != GetOwner()) return;

	// Progredi qualquer objetivo que escute o passo físico do personagem
	// Por exemplo, uma missão de "Caminhar X passos" associada a uma tag do objetivo
	for (const FSBActiveQuest& ActiveQuest : ActiveQuests)
	{
		if (ActiveQuest.bIsCompleted) continue;

		for (const FSBQuestObjective& Objective : ActiveQuest.QuestData->Objectives)
		{
			if (Objective.ObjectiveTag.IsValid() && Objective.ObjectiveTag.ToString().Contains(TEXT("Footstep")))
			{
				ProgressObjective(Objective.ObjectiveTag, 1);
			}
		}
	}
}

bool USBQuestComponent::SaveComponentData_Implementation(UObject* SavePayload)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return false;

	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (Payload)
	{
		Payload->SerializeObject(GetPathName(), this);
		return true;
	}
	return false;
}

bool USBQuestComponent::LoadComponentData_Implementation(UObject* SavePayload)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return false;

	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (Payload)
	{
		Payload->DeserializeObject(GetPathName(), this);
		return true;
	}
	return false;
}

void USBQuestComponent::AcceptQuest(USBQuestDataAsset* Quest)
{
	if (!Quest) return;

	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();

	if (!bIsServer && !GIsAutomationTesting) return;

	if (IsQuestActive(Quest)) return;

	FSBActiveQuest NewQuest;
	NewQuest.QuestData = Quest;
	NewQuest.bIsCompleted = false;
	NewQuest.bRewardsClaimed = false;

	for (const FSBQuestObjective& Objective : Quest->Objectives)
	{
		FSBQuestObjectiveProgress Progress;
		Progress.ObjectiveTag = Objective.ObjectiveTag;
		Progress.CurrentCount = 0;
		NewQuest.ObjectivesProgress.Add(Progress);
	}

	ActiveQuests.Add(NewQuest);
}

void USBQuestComponent::ProgressObjective(FGameplayTag ObjectiveTag, int32 Amount)
{
	if (Amount <= 0) return;

	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();

	if (!bIsServer && !GIsAutomationTesting) return;

	bool bAnyProgressed = false;

	for (FSBActiveQuest& ActiveQuest : ActiveQuests)
	{
		if (ActiveQuest.bIsCompleted) continue;

		for (int32 i = 0; i < ActiveQuest.QuestData->Objectives.Num(); ++i)
		{
			const FSBQuestObjective& Objective = ActiveQuest.QuestData->Objectives[i];
			if (Objective.ObjectiveTag == ObjectiveTag)
			{
				FSBQuestObjectiveProgress& Progress = ActiveQuest.ObjectivesProgress[i];
				if (Progress.CurrentCount < Objective.RequiredCount)
				{
					Progress.CurrentCount = FMath::Min(Progress.CurrentCount + Amount, Objective.RequiredCount);
					bAnyProgressed = true;

					OnQuestObjectiveProgressed.Broadcast(ActiveQuest.QuestData, ObjectiveTag);

					// Verifica se concluiu todos os objetivos da missão
					bool bAllDone = true;
					for (int32 j = 0; j < ActiveQuest.QuestData->Objectives.Num(); ++j)
					{
						if (ActiveQuest.ObjectivesProgress[j].CurrentCount < ActiveQuest.QuestData->Objectives[j].RequiredCount)
						{
							bAllDone = false;
							break;
						}
					}

					if (bAllDone)
					{
						ActiveQuest.bIsCompleted = true;
						OnQuestCompleted.Broadcast(ActiveQuest.QuestData);
					}
				}
			}
		}
	}

}

bool USBQuestComponent::IsQuestActive(USBQuestDataAsset* Quest) const
{
	if (!Quest) return false;

	for (const FSBActiveQuest& ActiveQuest : ActiveQuests)
	{
		if (ActiveQuest.QuestData == Quest)
		{
			return true;
		}
	}
	return false;
}

bool USBQuestComponent::IsQuestCompleted(USBQuestDataAsset* Quest) const
{
	if (!Quest) return false;

	for (const FSBActiveQuest& ActiveQuest : ActiveQuests)
	{
		if (ActiveQuest.QuestData == Quest)
		{
			return ActiveQuest.bIsCompleted;
		}
	}
	return false;
}

bool USBQuestComponent::ClaimQuestRewards(USBQuestDataAsset* Quest)
{
	if (!Quest) return false;

	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();

	if (!bIsServer && !GIsAutomationTesting) return false;

	for (FSBActiveQuest& ActiveQuest : ActiveQuests)
	{
		if (ActiveQuest.QuestData == Quest)
		{
			if (ActiveQuest.bIsCompleted && !ActiveQuest.bRewardsClaimed)
			{
				ActiveQuest.bRewardsClaimed = true;

				// 1. Concede recompensa de XP
				if (USBExperienceComponent* XPComp = Owner->FindComponentByClass<USBExperienceComponent>())
				{
					float TotalXP = 0.0f;
					for (const FSBQuestReward& Reward : Quest->Rewards)
					{
						TotalXP += Reward.Experience;
					}
					if (TotalXP > 0.0f)
					{
						XPComp->AddExperience(FMath::RoundToInt(TotalXP));
					}
				}

				// 2. Concede recompensa de itens via barramento de eventos (Desacoplado)
				UGameInstance* GI = Owner->GetGameInstance();
				USBEventSubsystem* EventSubsystem = GI ? GI->GetSubsystem<USBEventSubsystem>() : nullptr;

				if (EventSubsystem)
				{
					USBQuestRewardsPayload* Payload = NewObject<USBQuestRewardsPayload>(EventSubsystem);
					Payload->TargetPawn = Cast<APawn>(Owner);
					Payload->QuestData = Quest;

					EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Quest_RewardsClaimed, Payload);
				}

				return true;
			}
			break;
		}
	}

	return false;
}
