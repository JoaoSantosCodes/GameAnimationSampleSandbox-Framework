#include "Weapons/SBWeaponBehaviorHitscan.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Subsystems/SBLagCompensationSubsystem.h"

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

	// 1. Obtém a latência do cliente (Ping) e calcula o tempo de rebobinamento (One-Way Delay = Ping / 2)
	float PingSeconds = 0.0f;
	if (APlayerState* PlayerState = Character->GetPlayerState())
	{
		PingSeconds = (PlayerState->GetPingInMilliseconds() * 0.001f) * 0.5f;
	}

	// Limita a compensação a no máximo 500ms por motivos de segurança anti-cheat
	PingSeconds = FMath::Clamp(PingSeconds, 0.0f, 0.5f);

	float CurrentTime = World->GetTimeSeconds();
	float TargetTime = CurrentTime - PingSeconds;

	// 2. Rebobina temporariamente a localização e rotação de todos os personagens no servidor
	USBLagCompensationSubsystem* LagCompSubsystem = World->GetSubsystem<USBLagCompensationSubsystem>();
	TMap<TWeakObjectPtr<ACharacter>, FTransform> OriginalTransforms;

	if (LagCompSubsystem && PingSeconds > 0.0f)
	{
		LagCompSubsystem->RewindPositions(TargetTime, OriginalTransforms);
	}

	// 3. Determina a direção e alcance do disparo (Trace simplificado do local do olho do personagem)
	FVector EyeLocation;
	FRotator EyeRotation;
	Character->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector TraceStart = EyeLocation;
	float Range = 5000.f; // 50 metros alcance padrão
	FVector TraceEnd = TraceStart + (EyeRotation.Vector() * Range);

	FHitResult HitResult;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponHitscan), true, Character);

	bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	// 4. Restaura imediatamente as posições originais no servidor para manter o presente síncrono
	if (LagCompSubsystem && OriginalTransforms.Num() > 0)
	{
		LagCompSubsystem->RestorePositions(OriginalTransforms);
	}

	// 5. Processa o acerto e aplica dano autoritativo
	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		USBAttributeComponent* HitAttrComp = HitActor->FindComponentByClass<USBAttributeComponent>();
		
		if (HitAttrComp && WeaponDefinition)
		{
			FGameplayTag HealthTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Health"));
			float CurrentHealth = HitAttrComp->GetAttributeValue(HealthTag);
			float NewHealth = FMath::Max(0.0f, CurrentHealth - WeaponDefinition->Damage);
			HitAttrComp->SetAttributeBaseValue(HealthTag, NewHealth);
		}
	}
}
