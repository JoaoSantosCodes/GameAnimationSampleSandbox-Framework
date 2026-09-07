// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "SBPhysicalProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class SANDBOXCOMBAT_API ASBPhysicalProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASBPhysicalProjectile();

	// Inicializa os parâmetros de combate do projétil autoritativamente no servidor
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sandbox|Combat")
	void InitializeProjectile(
		float InDamage,
		float InSpeed,
		float InGravityScale,
		float InCritMultiplier,
		const TArray<FName>& InCritBones,
		float InRadialDamageRadius,
		AActor* InInstigatorActor
	);

	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat")
	float GetDamage() const { return Damage; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat")
	float GetRadialDamageRadius() const { return RadialDamageRadius; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float Damage = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float CriticalDamageMultiplier = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<FName> CriticalBoneNames;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float RadialDamageRadius = 0.0f;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> SourceInstigatorActor = nullptr;

	virtual void ProcessDirectHit(AActor* HitActor, const FHitResult& Hit);
	virtual void ProcessRadialDamage(const FVector& ExplosionCenter);
};
