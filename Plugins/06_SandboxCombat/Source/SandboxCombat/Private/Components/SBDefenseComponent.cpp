// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBDefenseComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBDefenseComponent::USBDefenseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBDefenseComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedAttributeComp = Owner->FindComponentByClass<USBAttributeComponent>();
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBDefenseComponent::OnShutdown_Implementation()
{
	StopBlocking();
}

void USBDefenseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (bIsBlocking)
	{
		BlockTime += DeltaTime;
		if (BlockTime > Settings.ParryWindowDuration && CachedStateComp.IsValid())
		{
			CachedStateComp->RemoveTag(Tags.State_Combat_ParryWindow);
		}
	}

	if (CounterAttackTimer > 0.0f)
	{
		CounterAttackTimer -= DeltaTime;
		if (CounterAttackTimer <= 0.0f)
		{
			CounterAttackTimer = 0.0f;
			if (CachedStateComp.IsValid())
			{
				CachedStateComp->RemoveTag(Tags.State_Combat_CounterAttackReady);
			}
			OnCounterAttackWindowExpired.Broadcast();
		}
	}
}

void USBDefenseComponent::StartBlocking()
{
	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}
	if (!CachedAttributeComp.IsValid() && GetOwner())
	{
		CachedAttributeComp = GetOwner()->FindComponentByClass<USBAttributeComponent>();
	}

	bIsBlocking = true;
	BlockTime = 0.0f;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Combat_Blocking);
		CachedStateComp->AddTag(Tags.State_Combat_ParryWindow);
		CachedStateComp->RemoveTag(Tags.State_Combat_GuardBroken);
	}
}

void USBDefenseComponent::StopBlocking()
{
	bIsBlocking = false;
	BlockTime = 0.0f;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Combat_Blocking);
		CachedStateComp->RemoveTag(Tags.State_Combat_ParryWindow);
	}
}

bool USBDefenseComponent::IsParryActive() const
{
	return bIsBlocking && (BlockTime <= Settings.ParryWindowDuration);
}

ESBBlockResult USBDefenseComponent::ProcessIncomingDamage(float InDamage, AActor* Attacker, float& OutMitigatedDamage)
{
	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}
	if (!CachedAttributeComp.IsValid() && GetOwner())
	{
		CachedAttributeComp = GetOwner()->FindComponentByClass<USBAttributeComponent>();
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	// 1. Sem Bloqueio Ativo
	if (!bIsBlocking)
	{
		OutMitigatedDamage = InDamage;
		return ESBBlockResult::None;
	}

	// 2. Bloqueio Perfeito (Parry)
	if (IsParryActive())
	{
		OutMitigatedDamage = 0.0f;

		// Aplica atordoamento (Stagger) no atacante se presente
		if (IsValid(Attacker))
		{
			if (USBStateComponent* AttackerState = Attacker->FindComponentByClass<USBStateComponent>())
			{
				AttackerState->AddTag(Tags.State_Combat_Staggered);
			}
		}

		// Ativa janela de contra-ataque
		CounterAttackTimer = Settings.CounterAttackWindowDuration;
		if (CachedStateComp.IsValid())
		{
			CachedStateComp->AddTag(Tags.State_Combat_CounterAttackReady);
		}

		OnParrySuccess.Broadcast(Attacker);
		return ESBBlockResult::Parried;
	}

	// 3. Bloqueio Regular
	float CurrentStamina = CachedAttributeComp.IsValid() ? CachedAttributeComp->GetAttributeValue(Tags.Attribute_Stamina) : 100.0f;

	if (CurrentStamina >= Settings.BlockStaminaCost)
	{
		if (CachedAttributeComp.IsValid())
		{
			CachedAttributeComp->SetAttributeBaseValue(Tags.Attribute_Stamina, CurrentStamina - Settings.BlockStaminaCost);
		}

		OutMitigatedDamage = InDamage * (1.0f - Settings.BlockDamageReduction);
		OnBlockSuccess.Broadcast(InDamage, OutMitigatedDamage, Attacker);
		return ESBBlockResult::Blocked;
	}

	// 4. Quebra de Guarda (Estamina Esgotada)
	if (CachedAttributeComp.IsValid())
	{
		CachedAttributeComp->SetAttributeBaseValue(Tags.Attribute_Stamina, 0.0f);
	}

	OutMitigatedDamage = InDamage;
	StopBlocking();

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Combat_GuardBroken);
	}

	OnGuardBroken.Broadcast(Attacker);
	return ESBBlockResult::GuardBroken;
}

float USBDefenseComponent::ConsumeCounterAttack()
{
	if (IsCounterAttackReady())
	{
		CounterAttackTimer = 0.0f;
		if (CachedStateComp.IsValid())
		{
			CachedStateComp->RemoveTag(FSBGameplayTags::Get().State_Combat_CounterAttackReady);
		}
		return Settings.CounterAttackDamageMultiplier;
	}
	return 1.0f;
}
