#pragma once

#include "CoreMinimal.h"
#include "Items/SBItemFragment.h"
#include "Interfaces/SBEquippableInterface.h"
#include "SBItemFragment_Equippable.generated.h"

class USBAbility;

UCLASS()
class SANDBOXINVENTORY_API USBItemFragment_Equippable : public USBItemFragment, public ISBEquippableInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<USBAbility> AbilityToGrant;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<UObject> WeaponBehaviorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UPrimaryDataAsset> WeaponDefinitionAsset;

	// ISBEquippableInterface
	virtual UClass* GetWeaponBehaviorClass_Implementation() const override { return WeaponBehaviorClass; }
	virtual UPrimaryDataAsset* GetWeaponDefinitionAsset_Implementation() const override { return WeaponDefinitionAsset; }
};
