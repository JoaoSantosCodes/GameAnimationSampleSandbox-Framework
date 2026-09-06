#include "Components/SBTraumaInjuryComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBTraumaInjuryComponent::USBTraumaInjuryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InitDefaultLimbs();
}

void USBTraumaInjuryComponent::InitDefaultLimbs()
{
	TraumaData.Limbs.Empty();

	TArray<ESBBodyLimb> AllLimbs = {
		ESBBodyLimb::Head,
		ESBBodyLimb::Torso,
		ESBBodyLimb::LeftArm,
		ESBBodyLimb::RightArm,
		ESBBodyLimb::LeftLeg,
		ESBBodyLimb::RightLeg
	};

	for (ESBBodyLimb Limb : AllLimbs)
	{
		FSBLimbTrauma Data;
		Data.Limb = Limb;
		Data.LimbHealth = 100.0f;
		Data.bIsFractured = false;
		Data.bIsSplinted = false;
		Data.BleedType = ESBBleedType::None;
		Data.bTourniquetApplied = false;

		TraumaData.Limbs.Add(Limb, Data);
	}
}

void USBTraumaInjuryComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBTraumaInjuryComponent::SetupTraumaSystem(float InitialBloodVolume)
{
	InitDefaultLimbs();
	TraumaData.BloodVolume = FMath::Clamp(InitialBloodVolume, 0.5f, 5.0f);
	TraumaData.MaxBloodVolume = 5.0f;
	TraumaData.bInHypovolemicShock = false;
	SyncTags();
}

void USBTraumaInjuryComponent::InflictLimbDamage(ESBBodyLimb Limb, float Damage, bool bCanFracture, ESBBleedType Bleed)
{
	if (!TraumaData.Limbs.Contains(Limb))
	{
		return;
	}

	FSBLimbTrauma& Data = TraumaData.Limbs[Limb];
	Data.LimbHealth = FMath::Max(0.0f, Data.LimbHealth - FMath::Max(0.0f, Damage));

	if (bCanFracture && !Data.bIsFractured)
	{
		Data.bIsFractured = true;
		Data.bIsSplinted = false;
		OnLimbFractured.Broadcast(Limb);
	}

	if (Bleed != ESBBleedType::None && Data.BleedType < Bleed)
	{
		Data.BleedType = Bleed;
		OnBleedStateChanged.Broadcast(Limb, Bleed);
	}

	SyncTags();
}

bool USBTraumaInjuryComponent::ApplySplint(ESBBodyLimb Limb)
{
	if (!TraumaData.Limbs.Contains(Limb))
	{
		return false;
	}

	FSBLimbTrauma& Data = TraumaData.Limbs[Limb];
	if (Data.bIsFractured && !Data.bIsSplinted)
	{
		Data.bIsSplinted = true;
		SyncTags();
		return true;
	}

	return false;
}

bool USBTraumaInjuryComponent::ApplyTourniquet(ESBBodyLimb Limb)
{
	if (Limb == ESBBodyLimb::Head || Limb == ESBBodyLimb::Torso)
	{
		return false;
	}

	if (!TraumaData.Limbs.Contains(Limb))
	{
		return false;
	}

	FSBLimbTrauma& Data = TraumaData.Limbs[Limb];
	if (!Data.bTourniquetApplied)
	{
		Data.bTourniquetApplied = true;
		OnTourniquetStateChanged.Broadcast(Limb, true);
		SyncTags();
		return true;
	}

	return false;
}

bool USBTraumaInjuryComponent::RemoveTourniquet(ESBBodyLimb Limb)
{
	if (!TraumaData.Limbs.Contains(Limb))
	{
		return false;
	}

	FSBLimbTrauma& Data = TraumaData.Limbs[Limb];
	if (Data.bTourniquetApplied)
	{
		Data.bTourniquetApplied = false;
		OnTourniquetStateChanged.Broadcast(Limb, false);
		SyncTags();
		return true;
	}

	return false;
}

void USBTraumaInjuryComponent::ApplyBandageOrSuture(ESBBodyLimb Limb)
{
	if (!TraumaData.Limbs.Contains(Limb))
	{
		return;
	}

	FSBLimbTrauma& Data = TraumaData.Limbs[Limb];
	if (Data.BleedType != ESBBleedType::None)
	{
		Data.BleedType = ESBBleedType::None;
		OnBleedStateChanged.Broadcast(Limb, ESBBleedType::None);
		SyncTags();
	}
}

void USBTraumaInjuryComponent::TransfuseBlood(float VolumeLiters)
{
	TraumaData.BloodVolume = FMath::Min(TraumaData.MaxBloodVolume, TraumaData.BloodVolume + FMath::Max(0.0f, VolumeLiters));

	if (TraumaData.bInHypovolemicShock && TraumaData.BloodVolume >= 3.8f)
	{
		TraumaData.bInHypovolemicShock = false;
		OnHypovolemicShockRecovered.Broadcast();
	}

	SyncTags();
}

FSBLimbTrauma USBTraumaInjuryComponent::GetLimbData(ESBBodyLimb Limb) const
{
	if (const FSBLimbTrauma* Found = TraumaData.Limbs.Find(Limb))
	{
		return *Found;
	}
	return FSBLimbTrauma();
}

bool USBTraumaInjuryComponent::IsLimbFractured(ESBBodyLimb Limb) const
{
	if (const FSBLimbTrauma* Found = TraumaData.Limbs.Find(Limb))
	{
		return Found->bIsFractured;
	}
	return false;
}

bool USBTraumaInjuryComponent::IsBleeding() const
{
	for (const auto& Pair : TraumaData.Limbs)
	{
		if (Pair.Value.BleedType != ESBBleedType::None && !Pair.Value.bTourniquetApplied)
		{
			return true;
		}
	}
	return false;
}

bool USBTraumaInjuryComponent::IsArterialBleeding() const
{
	for (const auto& Pair : TraumaData.Limbs)
	{
		if (Pair.Value.BleedType == ESBBleedType::Arterial && !Pair.Value.bTourniquetApplied)
		{
			return true;
		}
	}
	return false;
}

void USBTraumaInjuryComponent::SimulateTraumaTick(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	float TotalBloodLoss = 0.0f;

	for (auto& Pair : TraumaData.Limbs)
	{
		FSBLimbTrauma& Data = Pair.Value;

		if (Data.BleedType == ESBBleedType::None)
		{
			continue;
		}

		if (Data.bTourniquetApplied)
		{
			continue;
		}

		if (Data.BleedType == ESBBleedType::Venous)
		{
			TotalBloodLoss += 0.05f * DeltaTime;
		}
		else if (Data.BleedType == ESBBleedType::Arterial)
		{
			TotalBloodLoss += 0.3f * DeltaTime;
		}
	}

	TraumaData.BloodVolume = FMath::Max(0.0f, TraumaData.BloodVolume - TotalBloodLoss);

	// Hypovolemic Shock check
	if (!TraumaData.bInHypovolemicShock && TraumaData.BloodVolume < 3.5f)
	{
		TraumaData.bInHypovolemicShock = true;
		OnHypovolemicShockTriggered.Broadcast();
	}
	else if (TraumaData.bInHypovolemicShock && TraumaData.BloodVolume >= 3.8f)
	{
		TraumaData.bInHypovolemicShock = false;
		OnHypovolemicShockRecovered.Broadcast();
	}

	SyncTags();
}

void USBTraumaInjuryComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Trauma_Bleeding);
	CachedStateComp->RemoveTag(Tags.State_Trauma_ArterialBleed);
	CachedStateComp->RemoveTag(Tags.State_Trauma_Fracture_Arm);
	CachedStateComp->RemoveTag(Tags.State_Trauma_Fracture_Leg);
	CachedStateComp->RemoveTag(Tags.State_Trauma_TourniquetApplied);
	CachedStateComp->RemoveTag(Tags.State_Trauma_HypovolemicShock);

	bool bAnyBleed = false;
	bool bAnyArterial = false;
	bool bAnyTourniquet = false;
	bool bArmFractured = false;
	bool bLegFractured = false;

	for (const auto& Pair : TraumaData.Limbs)
	{
		const FSBLimbTrauma& Data = Pair.Value;

		if (Data.bTourniquetApplied)
		{
			bAnyTourniquet = true;
		}

		if (Data.BleedType != ESBBleedType::None && !Data.bTourniquetApplied)
		{
			bAnyBleed = true;
			if (Data.BleedType == ESBBleedType::Arterial)
			{
				bAnyArterial = true;
			}
		}

		if (Data.bIsFractured)
		{
			if (Data.Limb == ESBBodyLimb::LeftArm || Data.Limb == ESBBodyLimb::RightArm)
			{
				bArmFractured = true;
			}
			else if (Data.Limb == ESBBodyLimb::LeftLeg || Data.Limb == ESBBodyLimb::RightLeg)
			{
				bLegFractured = true;
			}
		}
	}

	if (bAnyBleed)
	{
		CachedStateComp->AddTag(Tags.State_Trauma_Bleeding);
	}

	if (bAnyArterial)
	{
		CachedStateComp->AddTag(Tags.State_Trauma_ArterialBleed);
	}

	if (bAnyTourniquet)
	{
		CachedStateComp->AddTag(Tags.State_Trauma_TourniquetApplied);
	}

	if (bArmFractured)
	{
		CachedStateComp->AddTag(Tags.State_Trauma_Fracture_Arm);
	}

	if (bLegFractured)
	{
		CachedStateComp->AddTag(Tags.State_Trauma_Fracture_Leg);
	}

	if (TraumaData.bInHypovolemicShock)
	{
		CachedStateComp->AddTag(Tags.State_Trauma_HypovolemicShock);
	}
}
