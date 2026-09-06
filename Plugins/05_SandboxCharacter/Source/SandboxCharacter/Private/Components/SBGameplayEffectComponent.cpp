#include "Components/SBGameplayEffectComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"

USBGameplayEffectComponent::USBGameplayEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBGameplayEffectComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedAttributeComp = Owner->FindComponentByClass<USBAttributeComponent>();
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBGameplayEffectComponent::OnShutdown_Implementation()
{
	ClearAllEffects();
}

void USBGameplayEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActiveEffects.Num() == 0) return;

	TArray<int32> ExpiredIndices;

	for (int32 i = 0; i < ActiveEffects.Num(); ++i)
	{
		FSBActiveGameplayEffect& Active = ActiveEffects[i];

		// 1. Ticks Periódicos (DoT / HoT)
		if (Active.Spec.Period > 0.0f)
		{
			Active.PeriodTimer += DeltaTime;
			if (Active.PeriodTimer >= Active.Spec.Period)
			{
				Active.PeriodTimer -= Active.Spec.Period;
				OnGameplayEffectPeriodicTick.Broadcast(Active.Spec, Active.CurrentStacks);

				if (CachedAttributeComp.IsValid())
				{
					for (const FSBGameplayEffectModifier& Mod : Active.Spec.Modifiers)
					{
						float CurrentVal = CachedAttributeComp->GetAttributeValue(Mod.AttributeTag);
						CachedAttributeComp->SetAttributeBaseValue(Mod.AttributeTag, CurrentVal + (Mod.Magnitude * Active.CurrentStacks));
					}
				}
			}
		}

		// 2. Decaimento de Duração
		if (Active.Spec.DurationType == ESBEffectDurationType::HasDuration)
		{
			Active.TimeRemaining -= DeltaTime;
			if (Active.TimeRemaining <= 0.0f)
			{
				ExpiredIndices.Add(i);
			}
		}
	}

	// Remove expirados em ordem reversa
	for (int32 k = ExpiredIndices.Num() - 1; k >= 0; --k)
	{
		int32 Index = ExpiredIndices[k];
		FSBGameplayEffectSpec ExpiredSpec = ActiveEffects[Index].Spec;
		int32 Stacks = ActiveEffects[Index].CurrentStacks;

		RemoveGrantedTags(ExpiredSpec);
		RemoveModifiersFromAttributes(ExpiredSpec, Stacks);
		ActiveEffects.RemoveAt(Index);

		OnGameplayEffectRemoved.Broadcast(ExpiredSpec);
	}
}

bool USBGameplayEffectComponent::ApplyGameplayEffectSpec(const FSBGameplayEffectSpec& Spec)
{
	if (!CachedAttributeComp.IsValid() && GetOwner())
	{
		CachedAttributeComp = GetOwner()->FindComponentByClass<USBAttributeComponent>();
	}
	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	// 1. Verificação de Imunidade
	if (CachedStateComp.IsValid() && Spec.ImmunityTags.Num() > 0)
	{
		if (CachedStateComp->HasAny(Spec.ImmunityTags))
		{
			return false;
		}
	}

	// 2. Purgação / Remoção de Efeitos Conflitantes
	if (Spec.RemoveEffectsWithTags.Num() > 0)
	{
		for (const FGameplayTag& PurgeTag : Spec.RemoveEffectsWithTags)
		{
			RemoveGameplayEffectByTag(PurgeTag);
		}
	}

	// 3. Efeito Instantâneo
	if (Spec.DurationType == ESBEffectDurationType::Instant)
	{
		if (CachedAttributeComp.IsValid())
		{
			for (const FSBGameplayEffectModifier& Mod : Spec.Modifiers)
			{
				float CurrentVal = CachedAttributeComp->GetAttributeValue(Mod.AttributeTag);
				CachedAttributeComp->SetAttributeBaseValue(Mod.AttributeTag, CurrentVal + Mod.Magnitude);
			}
		}
		OnGameplayEffectApplied.Broadcast(Spec, 1);
		return true;
	}

	// 4. Efeito Contínuo / Stacking
	FSBActiveGameplayEffect* Existing = ActiveEffects.FindByPredicate([&Spec](const FSBActiveGameplayEffect& Item)
	{
		return Item.Spec.EffectTag == Spec.EffectTag;
	});

	if (Existing)
	{
		int32 OldStacks = Existing->CurrentStacks;
		Existing->CurrentStacks = FMath::Min(Existing->CurrentStacks + 1, Spec.MaxStacks);
		Existing->TimeRemaining = Spec.Duration;

		ApplyModifiersToAttributes(Spec, OldStacks, Existing->CurrentStacks);
		OnGameplayEffectApplied.Broadcast(Spec, Existing->CurrentStacks);
		return true;
	}

	// 5. Novo Efeito Ativo
	FSBActiveGameplayEffect NewActive;
	NewActive.Spec = Spec;
	NewActive.TimeRemaining = Spec.Duration;
	NewActive.PeriodTimer = 0.0f;
	NewActive.CurrentStacks = 1;

	ApplyGrantedTags(Spec);
	ApplyModifiersToAttributes(Spec, 0, 1);
	ActiveEffects.Add(NewActive);

	OnGameplayEffectApplied.Broadcast(Spec, 1);
	return true;
}

bool USBGameplayEffectComponent::RemoveGameplayEffectByTag(FGameplayTag EffectTag)
{
	if (!EffectTag.IsValid()) return false;

	int32 Index = ActiveEffects.IndexOfByPredicate([EffectTag](const FSBActiveGameplayEffect& Item)
	{
		return Item.Spec.EffectTag == EffectTag || Item.Spec.EffectTag.MatchesTag(EffectTag);
	});

	if (Index != INDEX_NONE)
	{
		FSBGameplayEffectSpec Spec = ActiveEffects[Index].Spec;
		int32 Stacks = ActiveEffects[Index].CurrentStacks;

		RemoveGrantedTags(Spec);
		RemoveModifiersFromAttributes(Spec, Stacks);
		ActiveEffects.RemoveAt(Index);

		OnGameplayEffectRemoved.Broadcast(Spec);
		return true;
	}

	return false;
}

bool USBGameplayEffectComponent::HasActiveGameplayEffect(FGameplayTag EffectTag) const
{
	if (!EffectTag.IsValid()) return false;

	return ActiveEffects.ContainsByPredicate([EffectTag](const FSBActiveGameplayEffect& Item)
	{
		return Item.Spec.EffectTag == EffectTag || Item.Spec.EffectTag.MatchesTag(EffectTag);
	});
}

int32 USBGameplayEffectComponent::GetActiveEffectStacks(FGameplayTag EffectTag) const
{
	if (!EffectTag.IsValid()) return 0;

	const FSBActiveGameplayEffect* Found = ActiveEffects.FindByPredicate([EffectTag](const FSBActiveGameplayEffect& Item)
	{
		return Item.Spec.EffectTag == EffectTag || Item.Spec.EffectTag.MatchesTag(EffectTag);
	});

	return Found ? Found->CurrentStacks : 0;
}

void USBGameplayEffectComponent::ClearAllEffects()
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		FSBGameplayEffectSpec Spec = ActiveEffects[i].Spec;
		int32 Stacks = ActiveEffects[i].CurrentStacks;

		RemoveGrantedTags(Spec);
		RemoveModifiersFromAttributes(Spec, Stacks);
		OnGameplayEffectRemoved.Broadcast(Spec);
	}
	ActiveEffects.Empty();
}

void USBGameplayEffectComponent::ApplyModifiersToAttributes(const FSBGameplayEffectSpec& Spec, int32 OldStacks, int32 NewStacks)
{
	if (!CachedAttributeComp.IsValid()) return;

	for (const FSBGameplayEffectModifier& EffectMod : Spec.Modifiers)
	{
		// Se for efeito com Period (DoT/HoT), o dano/cura ocorre por tick e não como modificador estático
		if (Spec.Period > 0.0f) continue;

		// Restack substitui o modificador anterior desta mesma fonte. Sem isto, cada
		// aplicação deixava um modificador órfão acumulado no atributo.
		if (OldStacks > 0)
		{
			CachedAttributeComp->RemoveModifiersBySource(EffectMod.AttributeTag, Spec.EffectTag);
		}

		FSBAttributeModifier Mod;
		Mod.SourceTag = Spec.EffectTag;
		Mod.ModifierType = (EffectMod.ModifierOp == ESBEffectModifierOp::Multiply) ? ESBAttributeModifierType::Multiplicative : (EffectMod.ModifierOp == ESBEffectModifierOp::Override ? ESBAttributeModifierType::Override : ESBAttributeModifierType::Additive);
		// Magnitude permanece unitária: CalculateValueWithModifiers já multiplica por
		// StackCount ao agregar. Pré-multiplicar aqui aplicaria o stack duas vezes
		// (Magnitude x Stacks x Stacks).
		Mod.Magnitude = EffectMod.Magnitude;
		Mod.Duration = (Spec.DurationType == ESBEffectDurationType::HasDuration) ? Spec.Duration : 0.0f;
		Mod.StackCount = NewStacks;

		CachedAttributeComp->ApplyModifier(EffectMod.AttributeTag, Mod);
	}
}

void USBGameplayEffectComponent::RemoveModifiersFromAttributes(const FSBGameplayEffectSpec& Spec, int32 Stacks)
{
	if (!CachedAttributeComp.IsValid()) return;

	for (const FSBGameplayEffectModifier& EffectMod : Spec.Modifiers)
	{
		if (Spec.Period > 0.0f) continue;
		CachedAttributeComp->RemoveModifiersBySource(EffectMod.AttributeTag, Spec.EffectTag);
	}
}

void USBGameplayEffectComponent::ApplyGrantedTags(const FSBGameplayEffectSpec& Spec)
{
	if (!CachedStateComp.IsValid()) return;

	for (const FGameplayTag& Tag : Spec.GrantedTags)
	{
		CachedStateComp->AddTag(Tag);
	}
}

void USBGameplayEffectComponent::RemoveGrantedTags(const FSBGameplayEffectSpec& Spec)
{
	if (!CachedStateComp.IsValid()) return;

	for (const FGameplayTag& Tag : Spec.GrantedTags)
	{
		CachedStateComp->RemoveTag(Tag);
	}
}
