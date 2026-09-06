#pragma once

#include "CoreMinimal.h"
#include "Weapons/SBWeaponBehavior.h"
#include "SBWeaponBehaviorProjectile.generated.h"

class ASBPhysicalProjectile;

UCLASS(Blueprintable, BlueprintType)
class SANDBOXCOMBAT_API USBWeaponBehaviorProjectile : public USBWeaponBehavior
{
	GENERATED_BODY()

public:
	USBWeaponBehaviorProjectile();

	virtual void Enter_Implementation(const FSBBehaviorContext& Context) override;
	virtual void Exit_Implementation(const FSBBehaviorContext& Context) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Projectile")
	FGameplayTag WeaponStateTag;

	void SpawnProjectile(const FSBBehaviorContext& Context);
};
