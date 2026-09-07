// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBLockOnComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

USBLockOnComponent::USBLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBLockOnComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBLockOnComponent::OnShutdown_Implementation()
{
	UnlockTarget();
}

void USBLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsLockedOn())
	{
		AActor* Target = CurrentTarget.Get();
		AActor* Owner = GetOwner();

		if (!IsValid(Target) || !IsValid(Owner))
		{
			UnlockTarget();
			return;
		}

		float Distance = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
		if (Distance > Settings.BreakDistance)
		{
			UnlockTarget();
		}
	}
}

bool USBLockOnComponent::ToggleLockOn()
{
	if (IsLockedOn())
	{
		UnlockTarget();
		return false;
	}

	AActor* BestTarget = FindBestTarget();
	if (BestTarget)
	{
		return LockOnTarget(BestTarget);
	}
	return false;
}

bool USBLockOnComponent::LockOnTarget(AActor* Target)
{
	if (!IsValid(Target) || Target == GetOwner())
	{
		return false;
	}

	if (CurrentTarget.Get() == Target)
	{
		return true;
	}

	// Remove tag do alvo anterior se houver
	if (CurrentTarget.IsValid())
	{
		SetTargetStateTag(CurrentTarget.Get(), false);
	}

	CurrentTarget = Target;
	SetTargetStateTag(Target, true);

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Combat_LockedOn);
	}

	OnLockOnTargetChanged.Broadcast(Target);
	return true;
}

void USBLockOnComponent::UnlockTarget()
{
	if (CurrentTarget.IsValid())
	{
		SetTargetStateTag(CurrentTarget.Get(), false);
	}

	CurrentTarget = nullptr;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Combat_LockedOn);
	}

	OnLockOnTargetLost.Broadcast();
}

bool USBLockOnComponent::SwitchTarget(ESBLockOnSwitchDirection Direction)
{
	if (!IsLockedOn())
	{
		return ToggleLockOn();
	}

	TArray<FSBLockOnCandidate> Candidates;
	FindCandidates(Candidates);

	if (Candidates.Num() <= 1)
	{
		return false;
	}

	AActor* Current = CurrentTarget.Get();
	const FSBLockOnCandidate* CurrentCand = Candidates.FindByPredicate([Current](const FSBLockOnCandidate& Cand)
	{
		return Cand.TargetActor.Get() == Current;
	});

	float CurrentAngle = CurrentCand ? CurrentCand->SignedHorizontalAngle : 0.0f;

	AActor* BestSwitch = nullptr;
	float MinAngleDiff = TNumericLimits<float>::Max();

	for (const FSBLockOnCandidate& Cand : Candidates)
	{
		if (Cand.TargetActor.Get() == Current) continue;

		float Diff = Cand.SignedHorizontalAngle - CurrentAngle;

		if (Direction == ESBLockOnSwitchDirection::Right && Diff > 0.0f)
		{
			if (Diff < MinAngleDiff)
			{
				MinAngleDiff = Diff;
				BestSwitch = Cand.TargetActor.Get();
			}
		}
		else if (Direction == ESBLockOnSwitchDirection::Left && Diff < 0.0f)
		{
			float AbsDiff = FMath::Abs(Diff);
			if (AbsDiff < MinAngleDiff)
			{
				MinAngleDiff = AbsDiff;
				BestSwitch = Cand.TargetActor.Get();
			}
		}
	}

	if (BestSwitch)
	{
		return LockOnTarget(BestSwitch);
	}

	return false;
}

AActor* USBLockOnComponent::FindBestTarget() const
{
	TArray<FSBLockOnCandidate> Candidates;
	FindCandidates(Candidates);

	if (Candidates.Num() == 0) return nullptr;

	AActor* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	for (const FSBLockOnCandidate& Cand : Candidates)
	{
		float Score = (Cand.AngleDegrees * 1.5f) + ((Cand.Distance / Settings.LockDistance) * 50.0f);
		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Cand.TargetActor.Get();
		}
	}

	return BestTarget;
}

void USBLockOnComponent::FindCandidates(TArray<FSBLockOnCandidate>& OutCandidates) const
{
	OutCandidates.Empty();

	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->GetWorld()) return;

	FVector Origin = Owner->GetActorLocation();
	FVector Forward = Owner->GetActorForwardVector();
	FVector Right = Owner->GetActorRightVector();

	for (TActorIterator<AActor> It(Owner->GetWorld()); It; ++It)
	{
		AActor* CandidateActor = *It;
		if (!IsValid(CandidateActor) || CandidateActor == Owner) continue;

		// Considera apenas atores válidos de cena (que não sejam triggers ou subobjetos internos de level)
		if (CandidateActor->IsHidden() || CandidateActor->GetRootComponent() == nullptr) continue;

		FVector TargetLoc = CandidateActor->GetActorLocation();
		float Distance = FVector::Dist(Origin, TargetLoc);

		if (Distance > Settings.LockDistance || Distance < 10.0f) continue;

		FVector DirToTarget = (TargetLoc - Origin).GetSafeNormal();
		float Dot = FVector::DotProduct(Forward, DirToTarget);
		float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f)));

		if (AngleDegrees > (Settings.MaxAngleDegrees * 0.5f)) continue;

		// Line of Sight check
		if (Settings.bRequireLineOfSight)
		{
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Owner);
			Params.AddIgnoredActor(CandidateActor);

			bool bBlocked = Owner->GetWorld()->LineTraceSingleByChannel(
				Hit,
				Origin + FVector(0, 0, 50),
				TargetLoc + FVector(0, 0, 50),
				Settings.TraceChannel,
				Params
			);

			if (bBlocked) continue;
		}

		FSBLockOnCandidate Cand;
		Cand.TargetActor = CandidateActor;
		Cand.Distance = Distance;
		Cand.AngleDegrees = AngleDegrees;
		Cand.SignedHorizontalAngle = FVector::DotProduct(Right, DirToTarget);

		OutCandidates.Add(Cand);
	}
}

FRotator USBLockOnComponent::GetDesiredRotationToTarget() const
{
	if (!IsLockedOn() || !GetOwner()) return FRotator::ZeroRotator;

	FVector Origin = GetOwner()->GetActorLocation();
	FVector TargetLoc = CurrentTarget->GetActorLocation();

	return UKismetMathLibrary::FindLookAtRotation(Origin, TargetLoc);
}

void USBLockOnComponent::SetTargetStateTag(AActor* Target, bool bIsTarget)
{
	if (IsValid(Target))
	{
		if (USBStateComponent* StateComp = Target->FindComponentByClass<USBStateComponent>())
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			if (bIsTarget)
			{
				StateComp->AddTag(Tags.State_Combat_Target);
			}
			else
			{
				StateComp->RemoveTag(Tags.State_Combat_Target);
			}
		}
	}
}
