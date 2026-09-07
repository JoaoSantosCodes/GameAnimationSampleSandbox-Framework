// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBFootIKComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBFootIKComponent::USBFootIKComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBFootIKComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBFootIKComponent::OnShutdown_Implementation()
{
	ResetFootIK();
}

FRotator USBFootIKComponent::CalculateRotationFromNormal(const FVector& HitNormal) const
{
	FVector Normal = HitNormal.GetSafeNormal();
	float Pitch = FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z));
	float Roll = FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z));
	return FRotator(Pitch, 0.0f, Roll);
}

FSBFootIKResult USBFootIKComponent::CalculateFootIK(float LeftFootHeight, const FVector& LeftNormal, float RightFootHeight, const FVector& RightNormal)
{
	if (!bIsIKEnabled)
	{
		ResetFootIK();
		return CurrentResult;
	}

	CurrentResult.LeftFoot.FootOffset = LeftFootHeight;
	CurrentResult.LeftFoot.HitNormal = LeftNormal;
	CurrentResult.LeftFoot.FootRotation = CalculateRotationFromNormal(LeftNormal);
	CurrentResult.LeftFoot.bHit = true;

	CurrentResult.RightFoot.FootOffset = RightFootHeight;
	CurrentResult.RightFoot.HitNormal = RightNormal;
	CurrentResult.RightFoot.FootRotation = CalculateRotationFromNormal(RightNormal);
	CurrentResult.RightFoot.bHit = true;

	CurrentResult.PelvisOffset = FMath::Min(LeftFootHeight, RightFootHeight);
	CurrentResult.bIsGrounded = true;

	float AngleLeft = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(LeftNormal.GetSafeNormal(), FVector::UpVector), -1.0f, 1.0f)));
	float AngleRight = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(RightNormal.GetSafeNormal(), FVector::UpVector), -1.0f, 1.0f)));
	CurrentResult.bIsOnSlope = (AngleLeft > Settings.SlopeThresholdDegrees || AngleRight > Settings.SlopeThresholdDegrees);

	SyncStateTags();
	OnFootIKUpdated.Broadcast(CurrentResult);
	return CurrentResult;
}

void USBFootIKComponent::SetIKEnabled(bool bEnabled)
{
	bIsIKEnabled = bEnabled;
	if (!bIsIKEnabled)
	{
		ResetFootIK();
	}
}

void USBFootIKComponent::ResetFootIK()
{
	CurrentResult = FSBFootIKResult();
	SyncStateTags();
	OnFootIKUpdated.Broadcast(CurrentResult);
}

void USBFootIKComponent::SyncStateTags()
{
	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	CachedStateComp->RemoveTag(Tags.State_Movement_FootIKActive);
	CachedStateComp->RemoveTag(Tags.State_Movement_OnSlope);

	if (bIsIKEnabled && (CurrentResult.LeftFoot.bHit || CurrentResult.RightFoot.bHit))
	{
		CachedStateComp->AddTag(Tags.State_Movement_FootIKActive);
		if (CurrentResult.bIsOnSlope)
		{
			CachedStateComp->AddTag(Tags.State_Movement_OnSlope);
		}
	}
}
