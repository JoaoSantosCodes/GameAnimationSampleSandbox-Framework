// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBDismembermentComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBDismembermentComponent::USBDismembermentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBDismembermentComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBDismembermentComponent::OnShutdown_Implementation()
{
	RegisteredLimbs.Empty();
}

void USBDismembermentComponent::RegisterLimbDefinition(const FSBLimbDismemberDefinition& Def)
{
	RegisteredLimbs.Add(Def.LimbType, Def);
}

bool USBDismembermentComponent::SeverLimb(const FSBSeverLimbRequest& Request, USkeletalMeshComponent* MeshComp)
{
	FSBLimbDismemberDefinition* Def = RegisteredLimbs.Find(Request.LimbType);
	if (!Def || Def->bIsSevered)
	{
		return false;
	}

	Def->bIsSevered = true;

	if (MeshComp && Def->BoneName != NAME_None)
	{
		MeshComp->HideBoneByName(Def->BoneName, PBO_None);
	}

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Combat_Dismembered);
		if (Request.LimbType == ESBLimbType::Head)
		{
			CachedStateComp->AddTag(Tags.Combat_Dismember_Head);
		}
		else if (Request.LimbType == ESBLimbType::LeftArm || Request.LimbType == ESBLimbType::RightArm)
		{
			CachedStateComp->AddTag(Tags.Combat_Dismember_Arm);
		}
		else if (Request.LimbType == ESBLimbType::LeftLeg || Request.LimbType == ESBLimbType::RightLeg)
		{
			CachedStateComp->AddTag(Tags.Combat_Dismember_Leg);
		}
	}

	FVector Impulse = Request.ImpulseDirection.GetSafeNormal() * Request.ImpulseStrength;
	OnLimbSevered.Broadcast(Request.LimbType, Impulse);
	return true;
}

bool USBDismembermentComponent::IsLimbSevered(ESBLimbType LimbType) const
{
	if (const FSBLimbDismemberDefinition* Def = RegisteredLimbs.Find(LimbType))
	{
		return Def->bIsSevered;
	}
	return false;
}

int32 USBDismembermentComponent::GetSeveredLimbCount() const
{
	int32 Count = 0;
	for (const auto& Pair : RegisteredLimbs)
	{
		if (Pair.Value.bIsSevered)
		{
			Count++;
		}
	}
	return Count;
}

bool USBDismembermentComponent::GetLimbDefinition(ESBLimbType LimbType, FSBLimbDismemberDefinition& OutDef) const
{
	if (const FSBLimbDismemberDefinition* Def = RegisteredLimbs.Find(LimbType))
	{
		OutDef = *Def;
		return true;
	}
	return false;
}

void USBDismembermentComponent::ResetDismemberment(USkeletalMeshComponent* MeshComp)
{
	for (auto& Pair : RegisteredLimbs)
	{
		Pair.Value.bIsSevered = false;
		if (MeshComp && Pair.Value.BoneName != NAME_None)
		{
			MeshComp->UnHideBoneByName(Pair.Value.BoneName);
		}
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Combat_Dismembered);
		CachedStateComp->RemoveTag(Tags.Combat_Dismember_Head);
		CachedStateComp->RemoveTag(Tags.Combat_Dismember_Arm);
		CachedStateComp->RemoveTag(Tags.Combat_Dismember_Leg);
	}
}
