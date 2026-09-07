// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBSurgeryProstheticsComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBSurgeryProstheticsComponent::USBSurgeryProstheticsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	TargetOperationDuration = 5.0f;
	SetupSurgicalComponent(100.0f);
}

void USBSurgeryProstheticsComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBSurgeryProstheticsComponent::SetupSurgicalComponent(float InitialOrganHealth)
{
	PatientData.OperationState = ESBSurgicalOperationState::Idle;
	PatientData.InstalledProsthetics.Empty();
	PatientData.OrganHealth = FMath::Clamp(InitialOrganHealth, 0.0f, 100.0f);
	PatientData.ImmunosuppressantLevel = 0.0f;
	PatientData.OperationProgress = 0.0f;
	PatientData.bIsAnesthetized = false;
	PatientData.bIsOrganRejectionRisk = false;
	TargetOperationDuration = 5.0f;
	SyncTags();
}

void USBSurgeryProstheticsComponent::AdministerAnesthesia(float Duration)
{
	PatientData.bIsAnesthetized = true;
	PatientData.OperationState = ESBSurgicalOperationState::PreOpAnesthesia;
	OnAnesthesiaStateChanged.Broadcast(true);
	SyncTags();
}

void USBSurgeryProstheticsComponent::StartSurgicalOperation(float EstimatedDuration)
{
	TargetOperationDuration = FMath::Max(0.1f, EstimatedDuration);
	PatientData.OperationProgress = 0.0f;
	PatientData.OperationState = ESBSurgicalOperationState::InSurgery;
	SyncTags();
}

void USBSurgeryProstheticsComponent::InstallProsthetic(ESBSurgicalLimbType Limb, ESBProstheticGrade Grade, float Efficiency)
{
	if (Limb == ESBSurgicalLimbType::None || Grade == ESBProstheticGrade::None)
	{
		return;
	}

	FSBProstheticLimb* Existing = PatientData.InstalledProsthetics.FindByPredicate([Limb](const FSBProstheticLimb& Item)
	{
		return Item.LimbType == Limb;
	});

	if (Existing)
	{
		Existing->Grade = Grade;
		Existing->Efficiency = Efficiency;
		Existing->StructuralDurability = 100.0f;
	}
	else
	{
		FSBProstheticLimb NewProsthetic;
		NewProsthetic.LimbType = Limb;
		NewProsthetic.Grade = Grade;
		NewProsthetic.Efficiency = Efficiency;
		NewProsthetic.StructuralDurability = 100.0f;
		PatientData.InstalledProsthetics.Add(NewProsthetic);
	}

	OnProstheticInstalled.Broadcast(Limb, Grade);
	SyncTags();
}

void USBSurgeryProstheticsComponent::PerformOrganTransplant(float RestoredHealth)
{
	PatientData.OrganHealth = FMath::Clamp(RestoredHealth, 0.0f, 100.0f);
	if (PatientData.ImmunosuppressantLevel < 0.2f)
	{
		PatientData.bIsOrganRejectionRisk = true;
		OnOrganRejectionWarning.Broadcast();
	}
	SyncTags();
}

void USBSurgeryProstheticsComponent::AdministerImmunosuppressant(float Potency)
{
	PatientData.ImmunosuppressantLevel = FMath::Clamp(PatientData.ImmunosuppressantLevel + Potency, 0.0f, 1.0f);
	if (PatientData.ImmunosuppressantLevel >= 0.2f)
	{
		PatientData.bIsOrganRejectionRisk = false;
	}
	SyncTags();
}

void USBSurgeryProstheticsComponent::SimulateSurgicalTick(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	if (PatientData.OperationState == ESBSurgicalOperationState::InSurgery)
	{
		PatientData.OperationProgress = FMath::Clamp(PatientData.OperationProgress + (DeltaTime / TargetOperationDuration), 0.0f, 1.0f);
		if (PatientData.OperationProgress >= 1.0f)
		{
			PatientData.OperationState = ESBSurgicalOperationState::PostOpRecovery;
			OnSurgicalOperationCompleted.Broadcast();
		}
		SyncTags();
	}
}

bool USBSurgeryProstheticsComponent::HasProsthetic(ESBSurgicalLimbType Limb) const
{
	return PatientData.InstalledProsthetics.ContainsByPredicate([Limb](const FSBProstheticLimb& Item)
	{
		return Item.LimbType == Limb;
	});
}

float USBSurgeryProstheticsComponent::GetOverallEfficiencyBonus() const
{
	float TotalBonus = 0.0f;
	for (const FSBProstheticLimb& Item : PatientData.InstalledProsthetics)
	{
		if (Item.Efficiency > 1.0f)
		{
			TotalBonus += (Item.Efficiency - 1.0f);
		}
	}
	return TotalBonus;
}

void USBSurgeryProstheticsComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Surgery_UnderAnesthesia);
	CachedStateComp->RemoveTag(Tags.State_Surgery_Operating);
	CachedStateComp->RemoveTag(Tags.State_Surgery_ProstheticInstalled);
	CachedStateComp->RemoveTag(Tags.State_Surgery_OrganRejection);
	CachedStateComp->RemoveTag(Tags.State_Surgery_CyberneticAugmented);

	if (PatientData.bIsAnesthetized)
	{
		CachedStateComp->AddTag(Tags.State_Surgery_UnderAnesthesia);
	}

	if (PatientData.OperationState == ESBSurgicalOperationState::InSurgery)
	{
		CachedStateComp->AddTag(Tags.State_Surgery_Operating);
	}

	if (PatientData.InstalledProsthetics.Num() > 0)
	{
		CachedStateComp->AddTag(Tags.State_Surgery_ProstheticInstalled);
	}

	if (PatientData.bIsOrganRejectionRisk)
	{
		CachedStateComp->AddTag(Tags.State_Surgery_OrganRejection);
	}

	bool bHasCybernetic = PatientData.InstalledProsthetics.ContainsByPredicate([](const FSBProstheticLimb& Item)
	{
		return Item.Grade == ESBProstheticGrade::CyberneticAugment;
	});

	if (bHasCybernetic)
	{
		CachedStateComp->AddTag(Tags.State_Surgery_CyberneticAugmented);
	}
}
