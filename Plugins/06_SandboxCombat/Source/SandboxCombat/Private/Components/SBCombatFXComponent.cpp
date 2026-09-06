#include "Components/SBCombatFXComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"
#include "Kismet/GameplayStatics.h"

USBCombatFXComponent::USBCombatFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBCombatFXComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBCombatFXComponent::OnShutdown_Implementation()
{
	DeactivateWeaponTrail();
}

void USBCombatFXComponent::ActivateWeaponTrail(const FSBWeaponTrailConfig& TrailConfig, USceneComponent* AttachComp)
{
	ActiveTrailConfig = TrailConfig;
	ActiveTrailConfig.bIsActive = true;

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Combat_WeaponTrailActive);
	}

	OnWeaponTrailStateChanged.Broadcast(true);
}

void USBCombatFXComponent::DeactivateWeaponTrail()
{
	if (!ActiveTrailConfig.bIsActive)
	{
		return;
	}

	ActiveTrailConfig.bIsActive = false;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Combat_WeaponTrailActive);
	}

	OnWeaponTrailStateChanged.Broadcast(false);
}

bool USBCombatFXComponent::SpawnImpactDecal(const FHitResult& HitResult, const FSBImpactDecalConfig& DecalConfig)
{
	FVector DecalLocation = HitResult.ImpactPoint.IsNearlyZero() ? HitResult.Location : HitResult.ImpactPoint;
	FRotator DecalRotation = HitResult.ImpactNormal.IsNearlyZero() ? FRotator(-90.0f, 0.0f, 0.0f) : (-HitResult.ImpactNormal).Rotation();

	FSBCombatFXRequest Req;
	Req.FXType = ESBCombatFXType::ImpactDecal;
	Req.Location = DecalLocation;
	Req.Rotation = DecalRotation;
	Req.DecalConfig = DecalConfig;
	SpawnedDecalHistory.Add(Req);

	UWorld* World = GetWorld();
	if (World && DecalConfig.DecalMaterial)
	{
		UGameplayStatics::SpawnDecalAtLocation(
			World,
			DecalConfig.DecalMaterial,
			DecalConfig.DecalSize,
			DecalLocation,
			DecalRotation,
			DecalConfig.LifeSpan
		);
	}

	OnImpactDecalSpawned.Broadcast(DecalLocation, DecalRotation);
	return true;
}

bool USBCombatFXComponent::PlaySocketParticle(FName SocketName, USceneComponent* AttachComp)
{
	FSBCombatFXRequest Req;
	Req.FXType = ESBCombatFXType::SocketEmitter;
	Req.SocketName = SocketName;
	Req.AttachToComponent = AttachComp;
	SpawnedDecalHistory.Add(Req);
	return true;
}
