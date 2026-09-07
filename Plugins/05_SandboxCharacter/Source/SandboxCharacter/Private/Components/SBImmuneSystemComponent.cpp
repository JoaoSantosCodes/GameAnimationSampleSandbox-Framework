// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBImmuneSystemComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBImmuneSystemComponent::USBImmuneSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBImmuneSystemComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBImmuneSystemComponent::SetupImmuneSystem(float InitialImmunityStrength)
{
	ImmuneData.BaseImmunityStrength = FMath::Max(0.1f, InitialImmunityStrength);
	ImmuneData.BodyFeverOffset = 0.0f;
	ImmuneData.ActiveInfections.Empty();
	ImmuneData.AcquiredImmunities.Empty();
	SyncTags();
}

bool USBImmuneSystemComponent::ExposeToPathogen(const FSBPathogenStrain& Strain, float InitialLoad)
{
	if (ImmuneData.AcquiredImmunities.Contains(Strain.PathogenID))
	{
		return false; // Immune!
	}

	for (const FSBActiveInfection& Inf : ImmuneData.ActiveInfections)
	{
		if (Inf.Strain.PathogenID == Strain.PathogenID)
		{
			return false; // Already infected
		}
	}

	FSBActiveInfection NewInf;
	NewInf.Strain = Strain;
	NewInf.PathogenLoad = FMath::Clamp(InitialLoad, 1.0f, 100.0f);
	NewInf.AntibodyCount = 0.0f;
	NewInf.TreatmentEffectiveness = 0.0f;
	NewInf.Stage = ESBInfectionStage::Incubating;

	ImmuneData.ActiveInfections.Add(NewInf);

	OnInfectionStageChanged.Broadcast(Strain.PathogenID, ESBInfectionStage::Healthy, ESBInfectionStage::Incubating);
	SyncTags();
	return true;
}

void USBImmuneSystemComponent::ApplyMedicalTreatment(FName PathogenID, float MedicinePower)
{
	for (FSBActiveInfection& Inf : ImmuneData.ActiveInfections)
	{
		if (Inf.Strain.PathogenID == PathogenID)
		{
			Inf.TreatmentEffectiveness += FMath::Max(0.0f, MedicinePower);
			break;
		}
	}
}

void USBImmuneSystemComponent::SetImmunityModifier(float Multiplier)
{
	ImmuneData.BaseImmunityStrength = FMath::Max(0.1f, Multiplier);
}

bool USBImmuneSystemComponent::IsInfectedWith(FName PathogenID) const
{
	for (const FSBActiveInfection& Inf : ImmuneData.ActiveInfections)
	{
		if (Inf.Strain.PathogenID == PathogenID)
		{
			return true;
		}
	}
	return false;
}

ESBInfectionStage USBImmuneSystemComponent::GetInfectionStage(FName PathogenID) const
{
	for (const FSBActiveInfection& Inf : ImmuneData.ActiveInfections)
	{
		if (Inf.Strain.PathogenID == PathogenID)
		{
			return Inf.Stage;
		}
	}
	return ESBInfectionStage::Healthy;
}

void USBImmuneSystemComponent::SimulateImmuneTick(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	bool bHasSymptomaticOrSevere = false;

	for (int32 i = ImmuneData.ActiveInfections.Num() - 1; i >= 0; --i)
	{
		FSBActiveInfection& Inf = ImmuneData.ActiveInfections[i];
		ESBInfectionStage OldStage = Inf.Stage;

		// Pathogen replication
		float Growth = Inf.Strain.Virulence * DeltaTime;
		Inf.PathogenLoad = FMath::Min(100.0f, Inf.PathogenLoad + Growth);

		// Immune response & treatment suppression
		float AntibodyProduction = ImmuneData.BaseImmunityStrength * 1.5f * DeltaTime;
		Inf.AntibodyCount = FMath::Min(100.0f, Inf.AntibodyCount + AntibodyProduction);

		float TotalSuppression = (Inf.AntibodyCount * 0.1f + Inf.TreatmentEffectiveness * 5.0f) * DeltaTime;
		Inf.PathogenLoad = FMath::Max(0.0f, Inf.PathogenLoad - TotalSuppression);

		// Stage transitions
		if (Inf.PathogenLoad <= 0.0f)
		{
			FName CuredID = Inf.Strain.PathogenID;
			ImmuneData.AcquiredImmunities.AddUnique(CuredID);

			OnInfectionStageChanged.Broadcast(CuredID, Inf.Stage, ESBInfectionStage::Immune);
			OnInfectionCured.Broadcast(CuredID);
			OnImmunityAcquired.Broadcast(CuredID);

			ImmuneData.ActiveInfections.RemoveAt(i);
			continue;
		}
		else if (Inf.AntibodyCount > Inf.PathogenLoad || Inf.TreatmentEffectiveness > 0.0f)
		{
			Inf.Stage = ESBInfectionStage::Recovering;
		}
		else if (Inf.PathogenLoad >= Inf.Strain.IncubationThreshold)
		{
			if (Inf.PathogenLoad >= 75.0f)
			{
				Inf.Stage = ESBInfectionStage::Severe;
			}
			else
			{
				Inf.Stage = ESBInfectionStage::Symptomatic;
			}
			bHasSymptomaticOrSevere = true;
		}
		else
		{
			Inf.Stage = ESBInfectionStage::Incubating;
		}

		if (OldStage != Inf.Stage)
		{
			OnInfectionStageChanged.Broadcast(Inf.Strain.PathogenID, OldStage, Inf.Stage);
		}
	}

	// Fever update
	if (bHasSymptomaticOrSevere)
	{
		if (ImmuneData.BodyFeverOffset == 0.0f)
		{
			ImmuneData.BodyFeverOffset = 2.0f;
			OnFeverTriggered.Broadcast(ImmuneData.BodyFeverOffset);
		}
	}
	else
	{
		ImmuneData.BodyFeverOffset = 0.0f;
	}

	SyncTags();
}

void USBImmuneSystemComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Immunity_Infected);
	CachedStateComp->RemoveTag(Tags.State_Immunity_Incubating);
	CachedStateComp->RemoveTag(Tags.State_Immunity_Fever);
	CachedStateComp->RemoveTag(Tags.State_Immunity_Symptomatic);
	CachedStateComp->RemoveTag(Tags.State_Immunity_Recovering);
	CachedStateComp->RemoveTag(Tags.State_Immunity_Immune);

	if (ImmuneData.ActiveInfections.Num() > 0)
	{
		CachedStateComp->AddTag(Tags.State_Immunity_Infected);

		for (const FSBActiveInfection& Inf : ImmuneData.ActiveInfections)
		{
			switch (Inf.Stage)
			{
			case ESBInfectionStage::Incubating:
				CachedStateComp->AddTag(Tags.State_Immunity_Incubating);
				break;
			case ESBInfectionStage::Symptomatic:
			case ESBInfectionStage::Severe:
				CachedStateComp->AddTag(Tags.State_Immunity_Symptomatic);
				break;
			case ESBInfectionStage::Recovering:
				CachedStateComp->AddTag(Tags.State_Immunity_Recovering);
				break;
			default:
				break;
			}
		}
	}

	if (ImmuneData.BodyFeverOffset > 0.0f)
	{
		CachedStateComp->AddTag(Tags.State_Immunity_Fever);
	}

	if (ImmuneData.AcquiredImmunities.Num() > 0)
	{
		CachedStateComp->AddTag(Tags.State_Immunity_Immune);
	}
}
