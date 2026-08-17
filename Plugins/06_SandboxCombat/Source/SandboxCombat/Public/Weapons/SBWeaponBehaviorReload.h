#pragma once

#include "CoreMinimal.h"
#include "Behaviors/SBGameplayBehavior.h"
#include "SBWeaponBehaviorReload.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SANDBOXCOMBAT_API USBWeaponBehaviorReload : public USBGameplayBehavior
{
	GENERATED_BODY()

public:
	USBWeaponBehaviorReload();

	virtual bool CanEnter_Implementation(const FSBBehaviorContext& Context) const override;
	virtual void Enter_Implementation(const FSBBehaviorContext& Context) override;
	virtual void Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context) override;
	virtual void Exit_Implementation(const FSBBehaviorContext& Context) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload")
	float ReloadDuration = 2.0f;

	UPROPERTY(Transient)
	float ReloadTimeElapsed = 0.0f;
};
