#include "Weapons/SBWeaponBehaviorProjectile.h"
#include "Weapons/SBPhysicalProjectile.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "Components/SBStateComponent.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "SBGameplayTags.h"

USBWeaponBehaviorProjectile::USBWeaponBehaviorProjectile()
{
	WeaponStateTag = FSBGameplayTags::Get().State_Weapon_Firing;
}

void USBWeaponBehaviorProjectile::Enter_Implementation(const FSBBehaviorContext& Context)
{
	Super::Enter_Implementation(Context);

	if (CombatStateComponent)
	{
		CombatStateComponent->AddTag(WeaponStateTag);
	}

	SpawnProjectile(Context);
}

void USBWeaponBehaviorProjectile::Exit_Implementation(const FSBBehaviorContext& Context)
{
	if (CombatStateComponent)
	{
		CombatStateComponent->RemoveTag(WeaponStateTag);
	}

	Super::Exit_Implementation(Context);
}

void USBWeaponBehaviorProjectile::SpawnProjectile(const FSBBehaviorContext& Context)
{
	if (!Context.GameplayContext || !Context.GameplayContext->Character) return;

	ACharacter* Character = Context.GameplayContext->Character;
	UWorld* World = Character->GetWorld();
	if (!World) return;

	// O spawn autoritativo do projétil ocorre apenas no Servidor
	if (!Character->HasAuthority()) return;

	if (!WeaponDefinition || !WeaponDefinition->ProjectileClass) return;

	FVector EyeLocation;
	FRotator EyeRotation;
	Character->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector MuzzleLocation = EyeLocation + (EyeRotation.Vector() * 60.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Character;
	SpawnParams.Owner = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASBPhysicalProjectile* Projectile = World->SpawnActor<ASBPhysicalProjectile>(
		WeaponDefinition->ProjectileClass,
		MuzzleLocation,
		EyeRotation,
		SpawnParams
	);

	if (Projectile)
	{
		Projectile->InitializeProjectile(
			WeaponDefinition->Damage,
			WeaponDefinition->ProjectileInitialSpeed,
			WeaponDefinition->ProjectileGravityScale,
			WeaponDefinition->CriticalDamageMultiplier,
			WeaponDefinition->CriticalBoneNames,
			WeaponDefinition->RadialDamageRadius,
			Character
		);

		// Emite Evento de Disparo no Event Bus
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USBEventSubsystem* EventSubsystem = GI->GetSubsystem<USBEventSubsystem>())
			{
				USBPawnEventPayload* Payload = NewObject<USBPawnEventPayload>(this);
				Payload->TargetPawn = Character;
				EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Weapon_Fire, Payload);
			}
		}
	}
}
