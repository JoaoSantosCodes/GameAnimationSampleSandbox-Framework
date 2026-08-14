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

	// Dano base aplicado autoritativamente pelo disparo no servidor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Combat")
	float Damage = 20.0f;

	// Cadência de disparo (cooldown em segundos entre tiros)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Combat")
	float FireRate = 0.2f;

	// Custo de munição por disparo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Attributes")
	float AmmoCost = 1.0f;

	// Custo de mana por disparo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Attributes")
	float ManaCost = 0.0f;
};
