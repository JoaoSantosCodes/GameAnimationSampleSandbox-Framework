// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Behaviors/SBGameplayBehaviorDefinition.h"
#include "Types/SBCommonTypes.h"
#include "SBWeaponBehaviorDefinition.generated.h"

UCLASS(BlueprintType)
class SANDBOXCOMBAT_API USBWeaponBehaviorDefinition : public USBGameplayBehaviorDefinition
{
	GENERATED_BODY()

public:

	USBWeaponBehaviorDefinition();

	// Dano base aplicado autoritativamente pelo disparo no servidor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Combat")
	float Damage = 20.0f;

	// Multiplicador de dano crítico (ex: headshots)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Combat")
	float CriticalDamageMultiplier = 2.0f;

	// Nomes dos ossos no esqueleto do alvo que acionam dano crítico
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Combat")
	TArray<FName> CriticalBoneNames;

	// Cadência de disparo (cooldown em segundos entre tiros)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Combat")
	float FireRate = 0.2f;

	// Custo de munição por disparo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Attributes")
	float AmmoCost = 1.0f;

	// Custo de mana por disparo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Attributes")
	float ManaCost = 0.0f;

	// Custo de durabilidade por uso/disparo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Attributes")
	float DurabilityCost = 1.0f;

	// Classe do Projétil Balístico (utilizado quando o behavior de disparo for baseado em projéteis)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Projectile")
	TSubclassOf<class ASBPhysicalProjectile> ProjectileClass;

	// Velocidade inicial do projétil
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Projectile", meta = (ClampMin = "100.0"))
	float ProjectileInitialSpeed = 3500.0f;

	// Escala de gravidade aplicada à trajetória balística (0.0 = sem gravidade, 1.0 = normal)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Projectile")
	float ProjectileGravityScale = 1.0f;

	// Raio de dano radial/splash (0.0 = apenas dano direto no impacto)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Projectile", meta = (ClampMin = "0.0"))
	float RadialDamageRadius = 0.0f;

	// Opcional: A classe do Actor da Arma Visual a ser spawnado
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Visual")
	TSubclassOf<AActor> WeaponActorClass;

	// Opcional: Socket padrão no esqueleto do personagem para quando a arma estiver empunhada
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Visual")
	FName ActiveSocketName;

	// Opcional: Socket padrão para quando a arma estiver guardada/holstered
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Visual")
	FName HolsterSocketName;
};
