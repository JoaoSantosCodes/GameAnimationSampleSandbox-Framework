#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SBItemDurabilityInterface.h"
#include "SBItemInstance.generated.h"

class USBItemDefinition;
class USBItemFragment;

UCLASS(BlueprintType, Blueprintable)
class SANDBOXINVENTORY_API USBItemInstance : public UObject, public ISBItemDurabilityInterface
{
	GENERATED_BODY()

public:
	USBItemInstance();

	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<const USBItemDefinition> ItemDef = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	int32 StackCount = 1;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	FGameplayTagContainer DynamicTags;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	float Durability = 100.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	int32 UpgradeLevel = 0;

	// ISBItemDurabilityInterface
	virtual float GetDurability_Implementation() const override { return Durability; }
	virtual void SetDurability_Implementation(float NewDurability) override;
	virtual void ConsumeDurability_Implementation(float Amount) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const USBItemFragment* FindFragmentByClass(TSubclassOf<USBItemFragment> FragmentClass) const;
};
