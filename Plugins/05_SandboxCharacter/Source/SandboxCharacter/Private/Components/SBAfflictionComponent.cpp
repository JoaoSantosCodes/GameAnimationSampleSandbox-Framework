// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBAfflictionComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBAfflictionComponent::USBAfflictionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetupAfflictionComponent(0.0f);
}

void USBAfflictionComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBAfflictionComponent::SetupAfflictionComponent(float InitialResistance)
{
	AfflictionState.ActiveAfflictions.Empty();
	AfflictionState.InoculationResistance = FMath::Clamp(InitialResistance, 0.0f, 1.0f);
	AfflictionState.bIsMotorImpaired = false;
	SyncTags();
}

void USBAfflictionComponent::ApplyAffliction(ESBAfflictionType Type, float Severity, float Duration)
{
	if (Type == ESBAfflictionType::None || Duration <= 0.0f)
	{
		return;
	}

	float EffectiveSeverity = Severity * (1.0f - AfflictionState.InoculationResistance);
	if (EffectiveSeverity <= 0.05f)
	{
		return;
	}

	FSBAfflictionData* Existing = AfflictionState.ActiveAfflictions.FindByPredicate([Type](const FSBAfflictionData& Item)
	{
		return Item.AfflictionType == Type;
	});

	if (Existing)
	{
		Existing->Severity = FMath::Max(Existing->Severity, EffectiveSeverity);
		Existing->DurationRemaining = FMath::Max(Existing->DurationRemaining, Duration);
		if (Type == ESBAfflictionType::MotorImpairment)
		{
			Existing->MotorImpairmentMultiplier = FMath::Clamp(1.0f - Existing->Severity, 0.0f, 1.0f);
		}
	}
	else
	{
		FSBAfflictionData NewData;
		NewData.AfflictionType = Type;
		NewData.Severity = FMath::Clamp(EffectiveSeverity, 0.0f, 1.0f);
		NewData.DurationRemaining = Duration;
		if (Type == ESBAfflictionType::MotorImpairment)
		{
			NewData.MotorImpairmentMultiplier = FMath::Clamp(1.0f - NewData.Severity, 0.0f, 1.0f);
		}
		else
		{
			NewData.MotorImpairmentMultiplier = 1.0f;
		}
		AfflictionState.ActiveAfflictions.Add(NewData);
	}

	bool bPrevImpaired = AfflictionState.bIsMotorImpaired;
	AfflictionState.bIsMotorImpaired = (GetMotorSpeedMultiplier() < 0.95f);
	if (bPrevImpaired != AfflictionState.bIsMotorImpaired)
	{
		OnMotorImpairmentStateChanged.Broadcast(AfflictionState.bIsMotorImpaired);
	}

	OnAfflictionApplied.Broadcast(Type, EffectiveSeverity);
	SyncTags();
}

void USBAfflictionComponent::ApplyNeutralizer(ESBNeutralizerType Neutralizer, float Potency)
{
	if (Neutralizer == ESBNeutralizerType::None || Potency <= 0.0f)
	{
		return;
	}

	TArray<ESBAfflictionType> RemovedTypes;

	for (int32 i = AfflictionState.ActiveAfflictions.Num() - 1; i >= 0; --i)
	{
		bool bCured = false;
		if (Neutralizer == ESBNeutralizerType::UniversalPanacea)
		{
			bCured = true;
		}
		else if (Neutralizer == ESBNeutralizerType::HerbalBalm && AfflictionState.ActiveAfflictions[i].AfflictionType == ESBAfflictionType::TissueDegradation)
		{
			bCured = true;
		}
		else if (Neutralizer == ESBNeutralizerType::SynthesizedAntidote && AfflictionState.ActiveAfflictions[i].AfflictionType == ESBAfflictionType::MotorImpairment)
		{
			bCured = true;
		}

		if (bCured)
		{
			RemovedTypes.Add(AfflictionState.ActiveAfflictions[i].AfflictionType);
			AfflictionState.ActiveAfflictions.RemoveAt(i);
		}
	}

	bool bPrevImpaired = AfflictionState.bIsMotorImpaired;
	AfflictionState.bIsMotorImpaired = (GetMotorSpeedMultiplier() < 0.95f);
	if (bPrevImpaired != AfflictionState.bIsMotorImpaired)
	{
		OnMotorImpairmentStateChanged.Broadcast(AfflictionState.bIsMotorImpaired);
	}

	for (ESBAfflictionType RemovedType : RemovedTypes)
	{
		OnAfflictionNeutralized.Broadcast(RemovedType);
	}

	SyncTags();
}

void USBAfflictionComponent::ApplyInoculation(float ResistanceBoost, float Duration)
{
	AfflictionState.InoculationResistance = FMath::Clamp(AfflictionState.InoculationResistance + ResistanceBoost, 0.0f, 1.0f);
	SyncTags();
}

void USBAfflictionComponent::SimulateAfflictionTick(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	TArray<ESBAfflictionType> ExpiredTypes;

	for (int32 i = AfflictionState.ActiveAfflictions.Num() - 1; i >= 0; --i)
	{
		AfflictionState.ActiveAfflictions[i].DurationRemaining -= DeltaTime;
		if (AfflictionState.ActiveAfflictions[i].DurationRemaining <= 0.0f)
		{
			ExpiredTypes.Add(AfflictionState.ActiveAfflictions[i].AfflictionType);
			AfflictionState.ActiveAfflictions.RemoveAt(i);
		}
	}

	bool bPrevImpaired = AfflictionState.bIsMotorImpaired;
	AfflictionState.bIsMotorImpaired = (GetMotorSpeedMultiplier() < 0.95f);
	if (bPrevImpaired != AfflictionState.bIsMotorImpaired)
	{
		OnMotorImpairmentStateChanged.Broadcast(AfflictionState.bIsMotorImpaired);
	}

	for (ESBAfflictionType ExpiredType : ExpiredTypes)
	{
		OnAfflictionNeutralized.Broadcast(ExpiredType);
	}

	SyncTags();
}

float USBAfflictionComponent::GetMotorSpeedMultiplier() const
{
	float Multiplier = 1.0f;
	for (const FSBAfflictionData& Item : AfflictionState.ActiveAfflictions)
	{
		Multiplier = FMath::Min(Multiplier, Item.MotorImpairmentMultiplier);
	}
	return Multiplier;
}

void USBAfflictionComponent::SyncTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	CachedStateComp->RemoveTag(Tags.State_Affliction_Impaired);
	CachedStateComp->RemoveTag(Tags.State_Affliction_Degradation);
	CachedStateComp->RemoveTag(Tags.State_Affliction_Paralyzed);
	CachedStateComp->RemoveTag(Tags.State_Affliction_Inoculated);

	if (AfflictionState.InoculationResistance >= 0.3f)
	{
		CachedStateComp->AddTag(Tags.State_Affliction_Inoculated);
	}

	if (AfflictionState.bIsMotorImpaired)
	{
		CachedStateComp->AddTag(Tags.State_Affliction_Impaired);
	}

	for (const FSBAfflictionData& Item : AfflictionState.ActiveAfflictions)
	{
		if (Item.AfflictionType == ESBAfflictionType::MotorImpairment)
		{
			if (Item.Severity >= 0.7f)
			{
				CachedStateComp->AddTag(Tags.State_Affliction_Paralyzed);
			}
		}
		else if (Item.AfflictionType == ESBAfflictionType::TissueDegradation)
		{
			CachedStateComp->AddTag(Tags.State_Affliction_Degradation);
		}
	}
}
