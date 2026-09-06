#include "Weapons/SBPhysicalProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/OverlapResult.h"
#include "SBGameplayTags.h"

ASBPhysicalProjectile::ASBPhysicalProjectile()
{
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(60.0f);
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(12.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->bReturnMaterialOnMove = true;
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASBPhysicalProjectile::OnProjectileHit);
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 3500.0f;
	ProjectileMovement->MaxSpeed = 3500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 1.0f;

	Damage = 25.0f;
	CriticalDamageMultiplier = 2.0f;
	CriticalBoneNames.Add(FName(TEXT("head")));
	CriticalBoneNames.Add(FName(TEXT("neck_01")));
	RadialDamageRadius = 0.0f;
}

void ASBPhysicalProjectile::InitializeProjectile(
	float InDamage,
	float InSpeed,
	float InGravityScale,
	float InCritMultiplier,
	const TArray<FName>& InCritBones,
	float InRadialDamageRadius,
	AActor* InInstigatorActor)
{
	if (!HasAuthority()) return;

	Damage = InDamage;
	CriticalDamageMultiplier = InCritMultiplier;
	CriticalBoneNames = InCritBones;
	RadialDamageRadius = InRadialDamageRadius;
	SourceInstigatorActor = InInstigatorActor;

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InSpeed;
		ProjectileMovement->MaxSpeed = InSpeed;
		ProjectileMovement->ProjectileGravityScale = InGravityScale;
		ProjectileMovement->Velocity = GetActorForwardVector() * InSpeed;
	}

	if (InInstigatorActor && CollisionComponent)
	{
		CollisionComponent->IgnoreActorWhenMoving(InInstigatorActor, true);
	}
}

void ASBPhysicalProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	if (OtherActor == this || OtherActor == SourceInstigatorActor.Get())
	{
		return;
	}

	if (RadialDamageRadius > 0.0f)
	{
		ProcessRadialDamage(Hit.ImpactPoint);
	}
	else if (OtherActor)
	{
		ProcessDirectHit(OtherActor, Hit);
	}

	Destroy();
}

void ASBPhysicalProjectile::ProcessDirectHit(AActor* HitActor, const FHitResult& Hit)
{
	if (!HitActor) return;

	USBAttributeComponent* HitAttrComp = HitActor->FindComponentByClass<USBAttributeComponent>();
	if (HitAttrComp)
	{
		float RawDamage = Damage;
		FName HitBone = Hit.BoneName;
		bool bIsCritical = false;

		if (HitBone != NAME_None && CriticalBoneNames.Contains(HitBone))
		{
			RawDamage *= CriticalDamageMultiplier;
			bIsCritical = true;
		}

		// Mitigação por Defesa
		FGameplayTag DefenseTag = FSBGameplayTags::Get().Attribute_Defense;
		float DefenseVal = 0.0f;
		if (DefenseTag.IsValid())
		{
			DefenseVal = HitAttrComp->GetAttributeValue(DefenseTag);
		}

		float FinalDamage = RawDamage;
		if (DefenseVal > 0.0f)
		{
			float MitigationRatio = 100.0f / (100.0f + DefenseVal);
			FinalDamage = FMath::Max(1.0f, RawDamage * MitigationRatio);
		}

		FGameplayTag HealthTag = FSBGameplayTags::Get().Attribute_Health;
		float CurrentHealth = HitAttrComp->GetAttributeValue(HealthTag);
		float NewHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);
		HitAttrComp->SetAttributeBaseValue(HealthTag, NewHealth);

		// Aciona Hit Reaction no alvo
		if (USBStateComponent* TargetStateComp = HitActor->FindComponentByClass<USBStateComponent>())
		{
			FGameplayTag HitReactTag = FSBGameplayTags::Get().State_Character_HitReacting;
			if (HitReactTag.IsValid())
			{
				TargetStateComp->AddTag(HitReactTag);
			}
		}

		// Publica evento de combate tipado
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (USBEventSubsystem* EventSubsystem = GI->GetSubsystem<USBEventSubsystem>())
				{
					USBHitReactEventPayload* HitPayload = NewObject<USBHitReactEventPayload>(this);
					HitPayload->TargetPawn = Cast<APawn>(HitActor);
					HitPayload->InstigatorActor = SourceInstigatorActor.Get();
					HitPayload->HitBoneName = HitBone;
					HitPayload->HitDirection = GetActorForwardVector();
					HitPayload->DamageDealt = FinalDamage;
					HitPayload->bIsCritical = bIsCritical;

					EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Combat_HitReact, HitPayload);

					if (bIsCritical)
					{
						EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Combat_CriticalHit, HitPayload);
					}
				}
			}
		}
	}
}

void ASBPhysicalProjectile::ProcessRadialDamage(const FVector& ExplosionCenter)
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<FOverlapResult> Overlaps;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(RadialDamageRadius);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ProjectileRadialDamage), false, this);
	if (SourceInstigatorActor.IsValid())
	{
		QueryParams.AddIgnoredActor(SourceInstigatorActor.Get());
	}

	World->OverlapMultiByChannel(Overlaps, ExplosionCenter, FQuat::Identity, ECC_Pawn, SphereShape, QueryParams);

	TSet<AActor*> AffectedActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* TargetActor = Overlap.GetActor();
		if (TargetActor && !AffectedActors.Contains(TargetActor) && TargetActor != SourceInstigatorActor.Get())
		{
			AffectedActors.Add(TargetActor);

			USBAttributeComponent* HitAttrComp = TargetActor->FindComponentByClass<USBAttributeComponent>();
			if (HitAttrComp)
			{
				float Distance = FVector::Dist(ExplosionCenter, TargetActor->GetActorLocation());
				float Falloff = FMath::Clamp(1.0f - (Distance / RadialDamageRadius), 0.1f, 1.0f);
				float RawDamage = Damage * Falloff;

				// Mitigação por Defesa
				FGameplayTag DefenseTag = FSBGameplayTags::Get().Attribute_Defense;
				float DefenseVal = 0.0f;
				if (DefenseTag.IsValid())
				{
					DefenseVal = HitAttrComp->GetAttributeValue(DefenseTag);
				}

				float FinalDamage = RawDamage;
				if (DefenseVal > 0.0f)
				{
					float MitigationRatio = 100.0f / (100.0f + DefenseVal);
					FinalDamage = FMath::Max(1.0f, RawDamage * MitigationRatio);
				}

				FGameplayTag HealthTag = FSBGameplayTags::Get().Attribute_Health;
				float CurrentHealth = HitAttrComp->GetAttributeValue(HealthTag);
				float NewHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);
				HitAttrComp->SetAttributeBaseValue(HealthTag, NewHealth);

				if (USBStateComponent* TargetStateComp = TargetActor->FindComponentByClass<USBStateComponent>())
				{
					FGameplayTag HitReactTag = FSBGameplayTags::Get().State_Character_HitReacting;
					if (HitReactTag.IsValid())
					{
						TargetStateComp->AddTag(HitReactTag);
					}
				}

				if (UGameInstance* GI = World->GetGameInstance())
				{
					if (USBEventSubsystem* EventSubsystem = GI->GetSubsystem<USBEventSubsystem>())
					{
						USBHitReactEventPayload* HitPayload = NewObject<USBHitReactEventPayload>(this);
						HitPayload->TargetPawn = Cast<APawn>(TargetActor);
						HitPayload->InstigatorActor = SourceInstigatorActor.Get();
						HitPayload->HitBoneName = NAME_None;
						HitPayload->HitDirection = (TargetActor->GetActorLocation() - ExplosionCenter).GetSafeNormal();
						HitPayload->DamageDealt = FinalDamage;
						HitPayload->bIsCritical = false;

						EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Combat_HitReact, HitPayload);
					}
				}
			}
		}
	}
}
