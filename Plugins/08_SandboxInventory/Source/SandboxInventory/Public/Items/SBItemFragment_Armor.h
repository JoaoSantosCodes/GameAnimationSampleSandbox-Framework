#pragma once

#include "CoreMinimal.h"
#include "Items/SBItemFragment.h"
#include "GameplayTagContainer.h"
#include "Types/SBCommonTypes.h"
#include "SBItemFragment_Armor.generated.h"

USTRUCT(BlueprintType)
struct SANDBOXINVENTORY_API FSBItemAttributeModifier
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FGameplayTag AttributeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	ESBAttributeModifierType ModifierType = ESBAttributeModifierType::Additive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	float Magnitude = 0.0f;
};

UCLASS()
class SANDBOXINVENTORY_API USBItemFragment_Armor : public USBItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FGameplayTag EquipmentSlotTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TArray<FSBItemAttributeModifier> ModifiersToGrant;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<AActor> VisualArmorMeshClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FName AttachSocketName;
};
