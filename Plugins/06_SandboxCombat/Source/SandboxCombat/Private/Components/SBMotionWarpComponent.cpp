// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBMotionWarpComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"

USBMotionWarpComponent::USBMotionWarpComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBMotionWarpComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBMotionWarpComponent::OnShutdown_Implementation()
{
	StopMotionWarp(true);
}

void USBMotionWarpComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (WarpState == ESBMotionWarpState::Warping)
	{
		AActor* Owner = GetOwner();
		if (!IsValid(Owner))
		{
			StopMotionWarp(true);
			return;
		}

		ElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(ElapsedTime / FMath::Max(0.001f, CurrentConfig.WarpDuration), 0.0f, 1.0f);
		float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

		FVector NewLoc = StartLocation;
		FRotator NewRot = StartRotation;
		CalculateWarpTransform(SmoothAlpha, NewLoc, NewRot);

		if (CurrentConfig.bWarpTranslation)
		{
			Owner->SetActorLocation(NewLoc, true);
		}

		if (CurrentConfig.bWarpRotation)
		{
			Owner->SetActorRotation(NewRot);
		}

		if (Alpha >= 1.0f)
		{
			StopMotionWarp(false);
		}
	}
}

void USBMotionWarpComponent::StartMotionWarp(const FSBMotionWarpTarget& Target, const FSBMotionWarpConfig& Config)
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return;

	if (!CachedStateComp.IsValid())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	CurrentTarget = Target;
	CurrentConfig = Config;
	StartLocation = Owner->GetActorLocation();
	StartRotation = Owner->GetActorRotation();

	// Calcula ponto de parada e rotação final
	if (Target.TargetActor.IsValid())
	{
		FVector TargetActorLoc = Target.TargetActor->GetActorLocation();
		FVector DirToTarget = (TargetActorLoc - StartLocation);
		DirToTarget.Z = 0.0f;
		float Distance = DirToTarget.Size();

		if (Distance <= Target.TargetOffsetDistance)
		{
			CalculatedTargetLocation = StartLocation;
		}
		else
		{
			DirToTarget.Normalize();
			float DesiredWarpDist = Distance - Target.TargetOffsetDistance;
			float ClampedWarpDist = FMath::Clamp(DesiredWarpDist, 0.0f, Config.MaxWarpDistance);
			CalculatedTargetLocation = StartLocation + (DirToTarget * ClampedWarpDist);
		}

		CalculatedTargetRotation = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetActorLoc);
		CalculatedTargetRotation.Pitch = 0.0f;
		CalculatedTargetRotation.Roll = 0.0f;
	}
	else
	{
		FVector DirToTarget = (Target.TargetLocation - StartLocation);
		DirToTarget.Z = 0.0f;
		float Distance = DirToTarget.Size();

		if (Distance <= Target.TargetOffsetDistance)
		{
			CalculatedTargetLocation = StartLocation;
		}
		else
		{
			DirToTarget.Normalize();
			float DesiredWarpDist = Distance - Target.TargetOffsetDistance;
			float ClampedWarpDist = FMath::Clamp(DesiredWarpDist, 0.0f, Config.MaxWarpDistance);
			CalculatedTargetLocation = StartLocation + (DirToTarget * ClampedWarpDist);
		}

		CalculatedTargetRotation = Target.TargetRotation;
	}

	ElapsedTime = 0.0f;
	WarpState = ESBMotionWarpState::Warping;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Combat_MotionWarping);
	}

	OnMotionWarpStarted.Broadcast();
}

void USBMotionWarpComponent::StartMotionWarpToActor(AActor* TargetActor, float Duration, float OffsetDistance)
{
	if (!IsValid(TargetActor)) return;

	FSBMotionWarpTarget Target;
	Target.TargetActor = TargetActor;
	Target.TargetOffsetDistance = OffsetDistance;

	FSBMotionWarpConfig Config;
	Config.WarpDuration = Duration;
	Config.bWarpTranslation = true;
	Config.bWarpRotation = true;

	StartMotionWarp(Target, Config);
}

void USBMotionWarpComponent::StopMotionWarp(bool bAborted)
{
	if (WarpState != ESBMotionWarpState::Warping) return;

	WarpState = bAborted ? ESBMotionWarpState::Aborted : ESBMotionWarpState::Completed;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Combat_MotionWarping);
	}

	if (bAborted)
	{
		OnMotionWarpAborted.Broadcast();
	}
	else
	{
		OnMotionWarpCompleted.Broadcast();
	}
}

void USBMotionWarpComponent::CalculateWarpTransform(float Alpha, FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation = FMath::Lerp(StartLocation, CalculatedTargetLocation, Alpha);
	OutRotation = FMath::Lerp(StartRotation, CalculatedTargetRotation, Alpha);
}
