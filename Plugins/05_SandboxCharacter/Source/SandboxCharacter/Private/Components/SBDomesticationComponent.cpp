#include "Components/SBDomesticationComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBDomesticationComponent::USBDomesticationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetupFauna(NAME_None, FSBCreatureGenetics());
}

void USBDomesticationComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBDomesticationComponent::SetupFauna(FName PreferredFood, const FSBCreatureGenetics& InitialGenetics)
{
	DomesticationData.DomesticationState = ESBFaunaDomesticationState::Wild;
	DomesticationData.ReproductiveStage = ESBFaunaReproductiveStage::NonBreeding;
	DomesticationData.TameProgress = 0.0f;
	DomesticationData.AffectionLevel = 0.0f;
	DomesticationData.PreferredFoodID = PreferredFood;
	DomesticationData.PregnancyProgress = 0.0f;
	DomesticationData.GestationDuration = 30.0f;
	DomesticationData.Genetics = InitialGenetics;
	DomesticationData.MateGenetics = FSBCreatureGenetics();
	SyncTags();
}

bool USBDomesticationComponent::FeedTamingFood(FName FoodID, float FoodNutrition)
{
	if (DomesticationData.DomesticationState == ESBFaunaDomesticationState::Domesticated)
	{
		DomesticationData.AffectionLevel = FMath::Clamp(DomesticationData.AffectionLevel + (FoodNutrition * 0.1f), 0.0f, 1.0f);
		SyncTags();
		return true;
	}

	if (DomesticationData.DomesticationState != ESBFaunaDomesticationState::Wild && DomesticationData.DomesticationState != ESBFaunaDomesticationState::Taming)
	{
		return false;
	}

	float Efficiency = (FoodID == DomesticationData.PreferredFoodID) ? 1.5f : 0.5f;
	float TameGain = FMath::Max(0.05f, FoodNutrition * 0.1f * Efficiency);

	ESBFaunaDomesticationState OldState = DomesticationData.DomesticationState;
	DomesticationData.TameProgress = FMath::Clamp(DomesticationData.TameProgress + TameGain, 0.0f, 1.0f);

	if (DomesticationData.TameProgress >= 1.0f)
	{
		DomesticationData.DomesticationState = ESBFaunaDomesticationState::Domesticated;
		DomesticationData.AffectionLevel = 0.5f;
		OnDomesticationStateChanged.Broadcast(OldState, ESBFaunaDomesticationState::Domesticated);
		OnCreatureTamed.Broadcast();
	}
	else if (DomesticationData.DomesticationState == ESBFaunaDomesticationState::Wild)
	{
		DomesticationData.DomesticationState = ESBFaunaDomesticationState::Taming;
		OnDomesticationStateChanged.Broadcast(OldState, ESBFaunaDomesticationState::Taming);
	}

	SyncTags();
	return true;
}

void USBDomesticationComponent::PetCreature(float AffectionBoost)
{
	if (DomesticationData.DomesticationState == ESBFaunaDomesticationState::Domesticated)
	{
		DomesticationData.AffectionLevel = FMath::Clamp(DomesticationData.AffectionLevel + FMath::Max(0.0f, AffectionBoost), 0.0f, 1.0f);
		SyncTags();
	}
}

bool USBDomesticationComponent::StartBreedingWith(const FSBCreatureGenetics& PartnerGenetics, float GestationTime)
{
	if (DomesticationData.DomesticationState != ESBFaunaDomesticationState::Domesticated || DomesticationData.ReproductiveStage != ESBFaunaReproductiveStage::NonBreeding)
	{
		return false;
	}

	DomesticationData.ReproductiveStage = ESBFaunaReproductiveStage::Pregnant;
	DomesticationData.MateGenetics = PartnerGenetics;
	DomesticationData.GestationDuration = FMath::Max(1.0f, GestationTime);
	DomesticationData.PregnancyProgress = 0.0f;

	SyncTags();
	return true;
}

bool USBDomesticationComponent::BirthOffspring(FSBCreatureGenetics& OutOffspringGenetics)
{
	if (DomesticationData.ReproductiveStage != ESBFaunaReproductiveStage::OffspringReady)
	{
		return false;
	}

	// Genetic recombination
	OutOffspringGenetics.SpeedModifier = (DomesticationData.Genetics.SpeedModifier + DomesticationData.MateGenetics.SpeedModifier) * 0.5f;
	OutOffspringGenetics.StaminaModifier = (DomesticationData.Genetics.StaminaModifier + DomesticationData.MateGenetics.StaminaModifier) * 0.5f;
	OutOffspringGenetics.WeightCapacityModifier = (DomesticationData.Genetics.WeightCapacityModifier + DomesticationData.MateGenetics.WeightCapacityModifier) * 0.5f;
	OutOffspringGenetics.Generation = FMath::Max(DomesticationData.Genetics.Generation, DomesticationData.MateGenetics.Generation) + 1;
	OutOffspringGenetics.MutationCount = DomesticationData.Genetics.MutationCount + DomesticationData.MateGenetics.MutationCount;

	if (OutOffspringGenetics.Generation > 1)
	{
		OutOffspringGenetics.SpeedModifier *= 1.05f;
		OutOffspringGenetics.MutationCount += 1;
	}

	OutOffspringGenetics.CoatColor = DomesticationData.Genetics.CoatColor;

	// Reset pregnancy
	DomesticationData.ReproductiveStage = ESBFaunaReproductiveStage::NonBreeding;
	DomesticationData.PregnancyProgress = 0.0f;
	DomesticationData.MateGenetics = FSBCreatureGenetics();

	OnOffspringBirthed.Broadcast(OutOffspringGenetics);
	SyncTags();
	return true;
}

void USBDomesticationComponent::SimulateFaunaTick(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	if (DomesticationData.ReproductiveStage == ESBFaunaReproductiveStage::Pregnant)
	{
		float ProgressGain = DeltaTime / DomesticationData.GestationDuration;
		DomesticationData.PregnancyProgress = FMath::Clamp(DomesticationData.PregnancyProgress + ProgressGain, 0.0f, 1.0f);

		if (DomesticationData.PregnancyProgress >= 1.0f)
		{
			DomesticationData.ReproductiveStage = ESBFaunaReproductiveStage::OffspringReady;
			OnPregnancyCompleted.Broadcast();
		}

		SyncTags();
	}
}

void USBDomesticationComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Fauna_Wild);
	CachedStateComp->RemoveTag(Tags.State_Fauna_Taming);
	CachedStateComp->RemoveTag(Tags.State_Fauna_Domesticated);
	CachedStateComp->RemoveTag(Tags.State_Fauna_Pregnant);
	CachedStateComp->RemoveTag(Tags.State_Fauna_Mountable);

	switch (DomesticationData.DomesticationState)
	{
	case ESBFaunaDomesticationState::Wild:
		CachedStateComp->AddTag(Tags.State_Fauna_Wild);
		break;
	case ESBFaunaDomesticationState::Taming:
		CachedStateComp->AddTag(Tags.State_Fauna_Taming);
		break;
	case ESBFaunaDomesticationState::Domesticated:
		CachedStateComp->AddTag(Tags.State_Fauna_Domesticated);
		break;
	default:
		break;
	}

	if (DomesticationData.ReproductiveStage == ESBFaunaReproductiveStage::Pregnant)
	{
		CachedStateComp->AddTag(Tags.State_Fauna_Pregnant);
	}

	if (DomesticationData.DomesticationState == ESBFaunaDomesticationState::Domesticated && DomesticationData.AffectionLevel >= 0.8f)
	{
		CachedStateComp->AddTag(Tags.State_Fauna_Mountable);
	}
}
