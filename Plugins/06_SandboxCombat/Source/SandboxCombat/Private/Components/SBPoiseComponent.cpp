#include "Components/SBPoiseComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBPoiseComponent::USBPoiseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	CurrentPoise = Settings.MaxPoise;
}

void USBPoiseComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
	CurrentPoise = Settings.MaxPoise;
}

void USBPoiseComponent::OnShutdown_Implementation()
{
	ResetPoise();
}

void USBPoiseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsPoiseBroken)
	{
		StaggerTimer -= DeltaTime;
		if (StaggerTimer <= 0.0f)
		{
			ResetPoise();
		}
	}
	else if (CurrentPoise < Settings.MaxPoise)
	{
		// O delay consome apenas a fração do tick que lhe cabe; o restante já regenera no
		// mesmo tick. Com um if/else excludente, o tick que zera o delay não regeneraria
		// nada e o tempo excedente seria descartado.
		float RemainingDelta = DeltaTime;

		if (RegenDelayTimer > 0.0f)
		{
			const float Consumed = FMath::Min(RegenDelayTimer, RemainingDelta);
			RegenDelayTimer -= Consumed;
			RemainingDelta -= Consumed;
		}

		if (RegenDelayTimer <= 0.0f && RemainingDelta > 0.0f)
		{
			float OldPoise = CurrentPoise;
			CurrentPoise = FMath::Min(Settings.MaxPoise, CurrentPoise + Settings.PoiseRegenRate * RemainingDelta);
			if (OldPoise < Settings.MaxPoise && CurrentPoise >= Settings.MaxPoise)
			{
				OnPoiseRecovered.Broadcast();
			}
		}
	}
}

void USBPoiseComponent::SetSuperArmor(bool bEnabled)
{
	Settings.bHasSuperArmor = bEnabled;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		if (bEnabled)
		{
			CachedStateComp->AddTag(Tags.State_Combat_SuperArmor);
		}
		else
		{
			CachedStateComp->RemoveTag(Tags.State_Combat_SuperArmor);
		}
	}
}

void USBPoiseComponent::ResetPoise()
{
	CurrentPoise = Settings.MaxPoise;
	bIsPoiseBroken = false;
	RegenDelayTimer = 0.0f;
	StaggerTimer = 0.0f;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Combat_PoiseBroken);
	}

	OnPoiseRecovered.Broadcast();
}

ESBHitReactionDirection USBPoiseComponent::CalculateHitDirection(const FVector& HitLocation) const
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return ESBHitReactionDirection::Front;

	FVector OwnerLoc = Owner->GetActorLocation();
	FVector DirFromHit = (HitLocation - OwnerLoc);
	DirFromHit.Z = 0.0f;

	if (DirFromHit.IsNearlyZero())
	{
		return ESBHitReactionDirection::Front;
	}
	DirFromHit.Normalize();

	FVector Forward = Owner->GetActorForwardVector();
	Forward.Z = 0.0f;
	Forward.Normalize();

	FVector Right = Owner->GetActorRightVector();
	Right.Z = 0.0f;
	Right.Normalize();

	float ForwardDot = FVector::DotProduct(Forward, DirFromHit);
	float RightDot = FVector::DotProduct(Right, DirFromHit);

	if (ForwardDot >= 0.707f)
	{
		return ESBHitReactionDirection::Front;
	}
	else if (ForwardDot <= -0.707f)
	{
		return ESBHitReactionDirection::Back;
	}
	else if (RightDot > 0.0f)
	{
		return ESBHitReactionDirection::Right;
	}
	else
	{
		return ESBHitReactionDirection::Left;
	}
}

void USBPoiseComponent::ReceivePoiseDamage(float Damage, const FVector& HitLocation, AActor* InstigatorActor, FSBHitReactionResult& OutResult)
{
	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	OutResult.Direction = CalculateHitDirection(HitLocation);
	OutResult.PoiseDamageApplied = Damage;

	switch (OutResult.Direction)
	{
	case ESBHitReactionDirection::Front:
		OutResult.ReactionTag = Tags.Combat_Reaction_Front;
		break;
	case ESBHitReactionDirection::Back:
		OutResult.ReactionTag = Tags.Combat_Reaction_Back;
		break;
	case ESBHitReactionDirection::Left:
		OutResult.ReactionTag = Tags.Combat_Reaction_Left;
		break;
	case ESBHitReactionDirection::Right:
		OutResult.ReactionTag = Tags.Combat_Reaction_Right;
		break;
	}

	// 1. Super Armor check
	bool bHasArmor = Settings.bHasSuperArmor || (CachedStateComp.IsValid() && CachedStateComp->HasTag(Tags.State_Combat_SuperArmor));
	if (bHasArmor)
	{
		OutResult.bAbsorbedBySuperArmor = true;
		OutResult.Intensity = ESBHitReactionIntensity::None;
		OutResult.bPoiseBroken = false;
		OnHitReactionTriggered.Broadcast(OutResult);
		return;
	}

	// 2. Poise Broken already
	if (bIsPoiseBroken)
	{
		OutResult.bPoiseBroken = true;
		OutResult.Intensity = ESBHitReactionIntensity::Heavy;
		OnHitReactionTriggered.Broadcast(OutResult);
		return;
	}

	// 3. Poise calculation
	CurrentPoise = FMath::Max(0.0f, CurrentPoise - Damage);
	RegenDelayTimer = Settings.PoiseRegenDelay;
	OnPoiseDamaged.Broadcast(CurrentPoise);

	if (CurrentPoise <= 0.0f)
	{
		bIsPoiseBroken = true;
		StaggerTimer = Settings.StaggerDuration;
		OutResult.bPoiseBroken = true;
		OutResult.Intensity = (Damage >= Settings.MaxPoise * 0.8f) ? ESBHitReactionIntensity::Knockdown : ESBHitReactionIntensity::Heavy;

		if (CachedStateComp.IsValid())
		{
			CachedStateComp->AddTag(Tags.State_Combat_PoiseBroken);
		}

		OnPoiseBroken.Broadcast(OutResult);
	}
	else
	{
		OutResult.bPoiseBroken = false;
		OutResult.Intensity = (Damage >= Settings.MaxPoise * 0.4f) ? ESBHitReactionIntensity::Heavy : ESBHitReactionIntensity::Light;
	}

	OnHitReactionTriggered.Broadcast(OutResult);
}
