#include "Components/SBGrappleComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBGrappleComponent::USBGrappleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBGrappleComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBGrappleComponent::OnShutdown_Implementation()
{
	ReleaseAnchorPoint(false);
}

bool USBGrappleComponent::AttachAnchorPoint(const FVector& InAnchorLocation, const FVector& InHitNormal)
{
	if (IsAttached())
	{
		return false;
	}

	AActor* Owner = GetOwner();
	const float Dist = Owner ? FVector::Distance(Owner->GetActorLocation(), InAnchorLocation) : 0.0f;
	if (Dist > Settings.MaxGrappleRange)
	{
		return false;
	}

	AnchorData.AnchorLocation = InAnchorLocation;
	AnchorData.HitNormal = InHitNormal;
	AnchorData.InitialCableLength = Dist;
	AnchorData.CurrentCableLength = Dist;
	AnchorData.bIsAttached = true;

	GrappleState = ESBGrappleState::Swinging;

	SyncStateTags();
	OnGrappleAnchored.Broadcast(InAnchorLocation);
	OnGrappleStateChanged.Broadcast(GrappleState);
	return true;
}

bool USBGrappleComponent::StartPull()
{
	if (!IsAttached())
	{
		return false;
	}

	GrappleState = ESBGrappleState::Pulling;

	SyncStateTags();
	OnGrappleStateChanged.Broadcast(GrappleState);
	return true;
}

bool USBGrappleComponent::StartSwing()
{
	if (!IsAttached())
	{
		return false;
	}

	GrappleState = ESBGrappleState::Swinging;

	SyncStateTags();
	OnGrappleStateChanged.Broadcast(GrappleState);
	return true;
}

bool USBGrappleComponent::ReleaseAnchorPoint(bool bApplyLaunchImpulse)
{
	if (!IsAttached())
	{
		return false;
	}

	FVector ExitVelocity = FVector::ZeroVector;
	if (bApplyLaunchImpulse && GetOwner())
	{
		const FVector Direction = (AnchorData.AnchorLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
		ExitVelocity = Direction * (Settings.PullSpeed * Settings.LaunchImpulseMultiplier);
	}

	AnchorData.bIsAttached = false;
	AnchorData.CurrentCableLength = 0.0f;
	AnchorData.InitialCableLength = 0.0f;
	GrappleState = ESBGrappleState::None;

	SyncStateTags();
	OnGrappleReleased.Broadcast(ExitVelocity);
	OnGrappleStateChanged.Broadcast(GrappleState);
	return true;
}

void USBGrappleComponent::UpdateGrapplePhysics(float DeltaTime, const FVector& CurrentActorLocation)
{
	if (!IsAttached())
	{
		return;
	}

	const float CurrentDist = FVector::Distance(CurrentActorLocation, AnchorData.AnchorLocation);
	AnchorData.CurrentCableLength = CurrentDist;

	if (GrappleState == ESBGrappleState::Pulling)
	{
		if (CurrentDist <= Settings.MinDetachDistance)
		{
			ReleaseAnchorPoint(true);
		}
	}
}

void USBGrappleComponent::SyncStateTags()
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
	CachedStateComp->RemoveTag(Tags.State_Movement_Grappling);
	CachedStateComp->RemoveTag(Tags.State_Movement_Grappling_Pulling);
	CachedStateComp->RemoveTag(Tags.State_Movement_Grappling_Swinging);

	switch (GrappleState)
	{
	case ESBGrappleState::Pulling:
		CachedStateComp->AddTag(Tags.State_Movement_Grappling);
		CachedStateComp->AddTag(Tags.State_Movement_Grappling_Pulling);
		break;
	case ESBGrappleState::Swinging:
		CachedStateComp->AddTag(Tags.State_Movement_Grappling);
		CachedStateComp->AddTag(Tags.State_Movement_Grappling_Swinging);
		break;
	case ESBGrappleState::Firing:
	case ESBGrappleState::Detaching:
	case ESBGrappleState::None:
	default:
		break;
	}
}
