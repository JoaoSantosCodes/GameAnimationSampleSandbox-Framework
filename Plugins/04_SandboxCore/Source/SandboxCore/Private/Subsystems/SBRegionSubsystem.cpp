// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBRegionSubsystem.h"
#include "SBGameplayTags.h"
#include "Interfaces/SBCharacterInterface.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "Kismet/GameplayStatics.h"

USBRegionSubsystem::USBRegionSubsystem()
{
}

void USBRegionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	HazardTickTimer = 0.0f;
}

void USBRegionSubsystem::Deinitialize()
{
	Super::Deinitialize();
	ActorPresenceMap.Empty();
}

void USBRegionSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HazardTickTimer += DeltaTime;
	// Aplica dano ambiental 1 vez por segundo
	if (HazardTickTimer >= 1.0f)
	{
		float Elapsed = HazardTickTimer;
		HazardTickTimer = 0.0f;

		// Limpa referências inválidas
		TArray<TWeakObjectPtr<AActor>> ToRemove;

		for (auto& Pair : ActorPresenceMap)
		{
			AActor* Actor = Pair.Key.Get();
			if (!IsValid(Actor))
			{
				ToRemove.Add(Pair.Key);
				continue;
			}

			// Procura por regiões de perigo com dano ambiental ativo
			for (const FSBRegionData& Region : Pair.Value.ActiveRegions)
			{
				if (Region.EnvironmentalDamagePerSecond > 0.0f)
				{
					float Dmg = Region.EnvironmentalDamagePerSecond * Elapsed;
					UGameplayStatics::ApplyDamage(Actor, Dmg, nullptr, nullptr, UDamageType::StaticClass());
				}
			}
		}

		for (const auto& Key : ToRemove)
		{
			ActorPresenceMap.Remove(Key);
		}
	}
}

TStatId USBRegionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USBRegionSubsystem, STATGROUP_Tickables);
}

void USBRegionSubsystem::NotifyActorEnteredRegion(AActor* Actor, const FSBRegionData& RegionData)
{
	if (!IsValid(Actor)) return;

	FSBActorRegionPresence& Presence = ActorPresenceMap.FindOrAdd(Actor);
	Presence.ActiveRegions.Add(RegionData);

	ApplyRegionTagsToActor(Actor, RegionData);
	OnActorEnteredRegion.Broadcast(Actor, RegionData);
}

void USBRegionSubsystem::NotifyActorExitedRegion(AActor* Actor, const FSBRegionData& RegionData)
{
	if (!IsValid(Actor)) return;

	if (FSBActorRegionPresence* Presence = ActorPresenceMap.Find(Actor))
	{
		int32 Index = Presence->ActiveRegions.IndexOfByPredicate([&RegionData](const FSBRegionData& Item)
		{
			return Item.RegionTag == RegionData.RegionTag;
		});

		if (Index != INDEX_NONE)
		{
			Presence->ActiveRegions.RemoveAt(Index);
		}

		RemoveRegionTagsFromActor(Actor, RegionData);
		OnActorExitedRegion.Broadcast(Actor, RegionData);

		if (Presence->ActiveRegions.Num() == 0)
		{
			ActorPresenceMap.Remove(Actor);
		}
	}
}

bool USBRegionSubsystem::GetActorCurrentRegion(const AActor* Actor, FSBRegionData& OutRegionData) const
{
	if (!IsValid(Actor)) return false;

	if (const FSBActorRegionPresence* Presence = ActorPresenceMap.Find(const_cast<AActor*>(Actor)))
	{
		if (Presence->ActiveRegions.Num() > 0)
		{
			OutRegionData = Presence->ActiveRegions.Last();
			return true;
		}
	}
	return false;
}

bool USBRegionSubsystem::IsActorInSafeZone(const AActor* Actor) const
{
	if (!IsValid(Actor)) return false;

	if (const FSBActorRegionPresence* Presence = ActorPresenceMap.Find(const_cast<AActor*>(Actor)))
	{
		for (const FSBRegionData& Region : Presence->ActiveRegions)
		{
			if (Region.bIsSafeZone)
			{
				return true;
			}
		}
	}
	return false;
}

bool USBRegionSubsystem::IsPvPAllowedForActor(const AActor* Actor) const
{
	if (!IsValid(Actor)) return false;

	if (const FSBActorRegionPresence* Presence = ActorPresenceMap.Find(const_cast<AActor*>(Actor)))
	{
		for (const FSBRegionData& Region : Presence->ActiveRegions)
		{
			if (Region.bIsPvPAllowed)
			{
				return true;
			}
		}
	}
	return false;
}

bool USBRegionSubsystem::IsActorInHazard(const AActor* Actor) const
{
	if (!IsValid(Actor)) return false;

	if (const FSBActorRegionPresence* Presence = ActorPresenceMap.Find(const_cast<AActor*>(Actor)))
	{
		for (const FSBRegionData& Region : Presence->ActiveRegions)
		{
			if (Region.EnvironmentalDamagePerSecond > 0.0f || Region.RegionTag.MatchesTag(FSBGameplayTags::Get().Zone_Type_Hazard))
			{
				return true;
			}
		}
	}
	return false;
}

void USBRegionSubsystem::ApplyRegionTagsToActor(AActor* Actor, const FSBRegionData& RegionData)
{
	if (!IsValid(Actor)) return;

	UActorComponent* StateComp = nullptr;
	if (Actor->Implements<USBCharacterInterface>())
	{
		StateComp = ISBCharacterInterface::Execute_GetStateComponent(Actor);
	}
	if (!StateComp)
	{
		// Antes pegava o PRIMEIRO componente qualquer do ator, o que quase nunca era o de
		// estado. Resolve por contrato.
		StateComp = Actor->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	}

	if (!StateComp) return;

	UFunction* AddTagFunc = StateComp->FindFunction(TEXT("AddTag"));
	if (!AddTagFunc) return;

	auto InvokeAddTag = [StateComp, AddTagFunc](FGameplayTag Tag)
	{
		if (!Tag.IsValid()) return;
		struct FAddTagParams { FGameplayTag Tag; } Params{ Tag };
		StateComp->ProcessEvent(AddTagFunc, &Params);
	};

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (RegionData.bIsSafeZone)
	{
		InvokeAddTag(Tags.State_Zone_Safe);
	}
	if (RegionData.bIsPvPAllowed)
	{
		InvokeAddTag(Tags.State_Zone_PvPAllowed);
	}
	if (RegionData.EnvironmentalDamagePerSecond > 0.0f || RegionData.RegionTag.MatchesTag(Tags.Zone_Type_Hazard))
	{
		InvokeAddTag(Tags.State_Zone_InHazard);
	}

	for (const FGameplayTag& ExtraTag : RegionData.AppliedStateTags)
	{
		InvokeAddTag(ExtraTag);
	}
}

void USBRegionSubsystem::RemoveRegionTagsFromActor(AActor* Actor, const FSBRegionData& RegionData)
{
	if (!IsValid(Actor)) return;

	UActorComponent* StateComp = nullptr;
	if (Actor->Implements<USBCharacterInterface>())
	{
		StateComp = ISBCharacterInterface::Execute_GetStateComponent(Actor);
	}
	if (!StateComp)
	{
		// Antes pegava o PRIMEIRO componente qualquer do ator, o que quase nunca era o de
		// estado. Resolve por contrato.
		StateComp = Actor->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	}

	if (!StateComp) return;

	UFunction* RemoveTagFunc = StateComp->FindFunction(TEXT("RemoveTag"));
	if (!RemoveTagFunc) return;

	auto InvokeRemoveTag = [StateComp, RemoveTagFunc](FGameplayTag Tag)
	{
		if (!Tag.IsValid()) return;
		struct FRemoveTagParams { FGameplayTag Tag; } Params{ Tag };
		StateComp->ProcessEvent(RemoveTagFunc, &Params);
	};

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (RegionData.bIsSafeZone && !IsActorInSafeZone(Actor))
	{
		InvokeRemoveTag(Tags.State_Zone_Safe);
	}
	if (RegionData.bIsPvPAllowed && !IsPvPAllowedForActor(Actor))
	{
		InvokeRemoveTag(Tags.State_Zone_PvPAllowed);
	}
	if (!IsActorInHazard(Actor))
	{
		InvokeRemoveTag(Tags.State_Zone_InHazard);
	}

	for (const FGameplayTag& ExtraTag : RegionData.AppliedStateTags)
	{
		InvokeRemoveTag(ExtraTag);
	}
}
