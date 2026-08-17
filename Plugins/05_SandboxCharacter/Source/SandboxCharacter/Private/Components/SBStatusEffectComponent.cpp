#include "Components/SBStatusEffectComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

USBStatusEffectComponent::USBStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(true);
}

void USBStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedAttributeComponent = GetOwner()->FindComponentByClass<USBAttributeComponent>();
	CachedStateComponent = GetOwner()->FindComponentByClass<USBStateComponent>();
	ActiveEffects.OwnerComponent = this;
}

void USBStatusEffectComponent::ApplyStatusEffect(const USBStatusEffectDefinition* Definition)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!Definition || !Definition->EffectTag.IsValid())
	{
		return;
	}

	if (!CachedStateComponent)
	{
		CachedStateComponent = GetOwner()->FindComponentByClass<USBStateComponent>();
	}
	if (!CachedAttributeComponent)
	{
		CachedAttributeComponent = GetOwner()->FindComponentByClass<USBAttributeComponent>();
	}

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Busca se o efeito já está ativo para renovação
	FSBStatusEffectEntry* Existing = nullptr;
	for (FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		if (Entry.EffectTag == Definition->EffectTag)
		{
			Existing = &Entry;
			break;
		}
	}

	if (Existing)
	{
		// Renova a expiração e reseta o trigger do período
		Existing->ExpiryTime = Definition->DefaultDuration > 0.0f ? (CurrentTime + Definition->DefaultDuration) : 0.0f;
		Existing->LastPeriodTriggerTime = CurrentTime;
		Existing->Duration = Definition->DefaultDuration;
		ActiveEffects.MarkItemDirty(*Existing);
	}
	else
	{
		// Cria nova entrada de efeito ativo
		FSBStatusEffectEntry NewEntry;
		NewEntry.EffectTag = Definition->EffectTag;
		NewEntry.ExpiryTime = Definition->DefaultDuration > 0.0f ? (CurrentTime + Definition->DefaultDuration) : 0.0f;
		NewEntry.LastPeriodTriggerTime = CurrentTime;
		NewEntry.Duration = Definition->DefaultDuration;
		NewEntry.Period = Definition->DefaultPeriod;
		NewEntry.Definition = Definition;

		ActiveEffects.Entries.Add(NewEntry);
		ActiveEffects.MarkItemDirty(ActiveEffects.Entries.Last());

		// Aplica Gameplay Tags de estado associadas
		if (CachedStateComponent)
		{
			for (auto It = Definition->GrantedTags.CreateConstIterator(); It; ++It)
			{
				CachedStateComponent->AddTag(*It);
			}
		}

		// Aplica Modificadores de Atributo associados
		if (CachedAttributeComponent)
		{
			for (const FSBStatusEffectModifier& ModEntry : Definition->AttributeModifiers)
			{
				FSBAttributeModifier AppliedMod = ModEntry.Modifier;
				AppliedMod.SourceTag = Definition->EffectTag;
				CachedAttributeComponent->ApplyModifier(ModEntry.AttributeTag, AppliedMod);
			}
		}
	}
}

void USBStatusEffectComponent::RemoveStatusEffect(FGameplayTag EffectTag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	int32 FoundIndex = INDEX_NONE;
	for (int32 i = 0; i < ActiveEffects.Entries.Num(); ++i)
	{
		if (ActiveEffects.Entries[i].EffectTag == EffectTag)
		{
			FoundIndex = i;
			break;
		}
	}

	if (FoundIndex != INDEX_NONE)
	{
		const USBStatusEffectDefinition* Def = ActiveEffects.Entries[FoundIndex].Definition;

		ActiveEffects.Entries.RemoveAt(FoundIndex);
		ActiveEffects.MarkArrayDirty();

		if (!CachedStateComponent)
		{
			CachedStateComponent = GetOwner()->FindComponentByClass<USBStateComponent>();
		}
		if (!CachedAttributeComponent)
		{
			CachedAttributeComponent = GetOwner()->FindComponentByClass<USBAttributeComponent>();
		}

		// Limpa as Gameplay Tags e Modificadores do efeito removido
		if (Def)
		{
			if (CachedStateComponent)
			{
				for (auto It = Def->GrantedTags.CreateConstIterator(); It; ++It)
				{
					CachedStateComponent->RemoveTag(*It);
				}
			}

			if (CachedAttributeComponent)
			{
				for (const FSBStatusEffectModifier& ModEntry : Def->AttributeModifiers)
				{
					CachedAttributeComponent->RemoveModifiersBySource(ModEntry.AttributeTag, Def->EffectTag);
				}
			}
		}
	}
}

bool USBStatusEffectComponent::HasStatusEffect(FGameplayTag EffectTag) const
{
	for (const FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		if (Entry.EffectTag == EffectTag)
		{
			return true;
		}
	}
	return false;
}

float USBStatusEffectComponent::GetEffectRemainingTime(FGameplayTag EffectTag) const
{
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (const FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		if (Entry.EffectTag == EffectTag)
		{
			if (Entry.Duration <= 0.0f)
			{
				return -1.0f; // Efeito permanente
			}
			return FMath::Max(0.0f, Entry.ExpiryTime - CurrentTime);
		}
	}
	return 0.0f;
}

void USBStatusEffectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBStatusEffectComponent, ActiveEffects);
}

void USBStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	TArray<FGameplayTag> EffectsToRemove;

	if (!CachedAttributeComponent)
	{
		CachedAttributeComponent = GetOwner()->FindComponentByClass<USBAttributeComponent>();
	}

	for (FSBStatusEffectEntry& Entry : ActiveEffects.Entries)
	{
		// 1. Processa ticks periódicos (DOT/HOT)
		if (Entry.Period > 0.0f && Entry.Definition)
		{
			while (CurrentTime - Entry.LastPeriodTriggerTime >= Entry.Period)
			{
				if (CachedAttributeComponent && Entry.Definition->PeriodAttributeTag.IsValid())
				{
					float CurrentVal = CachedAttributeComponent->GetAttributeValue(Entry.Definition->PeriodAttributeTag);
					float NewVal = CurrentVal + Entry.Definition->PeriodAttributeChange;
					CachedAttributeComponent->SetAttributeBaseValue(Entry.Definition->PeriodAttributeTag, NewVal);
				}

				Entry.LastPeriodTriggerTime += Entry.Period;
				ActiveEffects.MarkItemDirty(Entry);
			}
		}

		// 2. Registra efeitos expirados
		if (Entry.Duration > 0.0f && CurrentTime >= Entry.ExpiryTime)
		{
			EffectsToRemove.Add(Entry.EffectTag);
		}
	}

	// 3. Remove os efeitos expirados
	for (FGameplayTag Tag : EffectsToRemove)
	{
		RemoveStatusEffect(Tag);
	}
}
