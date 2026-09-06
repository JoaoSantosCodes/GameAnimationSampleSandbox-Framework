#include "Components/SBZiplineComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBZiplineComponent::USBZiplineComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBZiplineComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBZiplineComponent::OnShutdown_Implementation()
{
	DetachFromZipline(false);
}

bool USBZiplineComponent::AttachToZipline(const FVector& InStartPoint, const FVector& InEndPoint)
{
	if (IsRiding())
	{
		return false;
	}

	const float Dist = FVector::Distance(InStartPoint, InEndPoint);
	if (Dist <= 0.0f)
	{
		return false;
	}

	RideData.StartPoint = InStartPoint;
	RideData.EndPoint = InEndPoint;
	RideData.TotalDistance = Dist;
	RideData.CurrentDistance = 0.0f;
	RideData.CurrentSpeed = Settings.BaseSlideSpeed;
	RideData.bIsRiding = true;

	ZiplineState = ESBZiplineState::Sliding;

	SyncStateTags();
	OnZiplineStateChanged.Broadcast(ZiplineState);
	return true;
}

bool USBZiplineComponent::DetachFromZipline(bool bApplyLaunchImpulse)
{
	if (!IsRiding())
	{
		return false;
	}

	FVector ExitVelocity = FVector::ZeroVector;
	if (bApplyLaunchImpulse && RideData.TotalDistance > 0.0f)
	{
		const FVector CableDirection = (RideData.EndPoint - RideData.StartPoint).GetSafeNormal();
		ExitVelocity = CableDirection * (RideData.CurrentSpeed * Settings.DismountLaunchMultiplier);
	}

	RideData.bIsRiding = false;
	RideData.CurrentSpeed = 0.0f;
	RideData.CurrentDistance = 0.0f;
	RideData.TotalDistance = 0.0f;

	ZiplineState = ESBZiplineState::None;

	SyncStateTags();
	OnZiplineDismounted.Broadcast(ExitVelocity);
	OnZiplineStateChanged.Broadcast(ZiplineState);
	return true;
}

void USBZiplineComponent::UpdateZiplineTravel(float DeltaTime)
{
	if (!IsRiding() || RideData.TotalDistance <= 0.0f)
	{
		return;
	}

	const FVector CableVec = RideData.EndPoint - RideData.StartPoint;
	const float SlopeRatio = FMath::Clamp(-CableVec.Z / RideData.TotalDistance, 0.0f, 1.0f);

	RideData.CurrentSpeed = FMath::Clamp(RideData.CurrentSpeed + (Settings.GravityAcceleration * SlopeRatio * DeltaTime), Settings.BaseSlideSpeed, Settings.MaxSlideSpeed);
	RideData.CurrentDistance += RideData.CurrentSpeed * DeltaTime;

	const float Alpha = FMath::Clamp(RideData.CurrentDistance / RideData.TotalDistance, 0.0f, 1.0f);
	OnZiplineProgress.Broadcast(Alpha);

	if (RideData.CurrentDistance >= RideData.TotalDistance)
	{
		DetachFromZipline(true);
	}
}

FVector USBZiplineComponent::GetCalculatedLocation() const
{
	if (!IsRiding() || RideData.TotalDistance <= 0.0f)
	{
		return RideData.StartPoint;
	}

	const float Alpha = FMath::Clamp(RideData.CurrentDistance / RideData.TotalDistance, 0.0f, 1.0f);
	return FMath::Lerp(RideData.StartPoint, RideData.EndPoint, Alpha);
}

void USBZiplineComponent::SyncStateTags()
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
	CachedStateComp->RemoveTag(Tags.State_Movement_Ziplining);
	CachedStateComp->RemoveTag(Tags.State_Movement_Ziplining_Sliding);
	CachedStateComp->RemoveTag(Tags.State_Movement_Ziplining_Dismounting);

	switch (ZiplineState)
	{
	case ESBZiplineState::Sliding:
		CachedStateComp->AddTag(Tags.State_Movement_Ziplining);
		CachedStateComp->AddTag(Tags.State_Movement_Ziplining_Sliding);
		break;
	case ESBZiplineState::Mounting:
		CachedStateComp->AddTag(Tags.State_Movement_Ziplining);
		break;
	case ESBZiplineState::Dismounting:
		CachedStateComp->AddTag(Tags.State_Movement_Ziplining_Dismounting);
		break;
	case ESBZiplineState::None:
	default:
		break;
	}
}
