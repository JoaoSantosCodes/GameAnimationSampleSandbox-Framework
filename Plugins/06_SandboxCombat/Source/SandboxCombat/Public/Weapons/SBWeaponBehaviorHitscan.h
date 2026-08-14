#pragma once

#include "CoreMinimal.h"
#include "Weapons/SBWeaponBehavior.h"
#include "SBWeaponBehaviorHitscan.generated.h"

UCLASS(Blueprintable, BlueprintType)
class SANDBOXCOMBAT_API USBWeaponBehaviorHitscan : public USBWeaponBehavior
{
	GENERATED_BODY()

public:
	USBWeaponBehaviorHitscan();

	virtual void Enter_Implementation(const FSBBehaviorContext& Context) override;
	virtual void Exit_Implementation(const FSBBehaviorContext& Context) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Hitscan")
	FGameplayTag WeaponStateTag;

	void PerformHitscanTrace(const FSBBehaviorContext& Context);
};
