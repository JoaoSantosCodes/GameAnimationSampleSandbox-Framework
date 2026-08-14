#include "Weapons/SBWeaponBehaviorHitscan.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

USBWeaponBehaviorHitscan::USBWeaponBehaviorHitscan()
{
	WeaponStateTag = FGameplayTag::RequestGameplayTag(TEXT("State.Weapon.Firing"));
}

void USBWeaponBehaviorHitscan::Enter_Implementation(const FSBBehaviorContext& Context)
{
	Super::Enter_Implementation(Context);

	if (CombatStateComponent)
	{
		CombatStateComponent->AddTag(WeaponStateTag);
	}

	PerformHitscanTrace(Context);
}

void USBWeaponBehaviorHitscan::Exit_Implementation(const FSBBehaviorContext& Context)
{
	if (CombatStateComponent)
	{
		CombatStateComponent->RemoveTag(WeaponStateTag);
	}

	Super::Exit_Implementation(Context);
}

void USBWeaponBehaviorHitscan::PerformHitscanTrace(const FSBBehaviorContext& Context)
{
	if (!Context.GameplayContext || !Context.GameplayContext->Character) return;

	ACharacter* Character = Context.GameplayContext->Character;
	UWorld* World = Character->GetWorld();
	if (!World) return;

	// O disparo físico real (cálculo de traço e dano) roda apenas no Servidor
	if (!Character->HasAuthority()) return;

	// Determina a direção e alcance do disparo (Trace simplificado do local do olho do personagem)
	FVector EyeLocation;
	FRotator EyeRotation;
	Character->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector TraceStart = EyeLocation;
	float Range = 5000.f; // 50 metros alcance padrão
	FVector TraceEnd = TraceStart + (EyeRotation.Vector() * Range);

	FHitResult HitResult;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponHitscan), true, Character);

	bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		USBAttributeComponent* HitAttrComp = HitActor->FindComponentByClass<USBAttributeComponent>();
		
		if (HitAttrComp && WeaponDefinition)
		{
			FGameplayTag HealthTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Health"));
			// Aplica dano reduzindo o Health do alvo autoritativamente via SetAttributeBaseValue
			float CurrentHealth = HitAttrComp->GetAttributeValue(HealthTag);
			float NewHealth = FMath::Max(0.0f, CurrentHealth - WeaponDefinition->Damage);
			HitAttrComp->SetAttributeBaseValue(HealthTag, NewHealth);
		}
	}
}
