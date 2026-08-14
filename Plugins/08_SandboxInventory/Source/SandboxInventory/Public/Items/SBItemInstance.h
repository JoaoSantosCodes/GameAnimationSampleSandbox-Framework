#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "SBItemInstance.generated.h"

class USBItemDefinition;
class USBItemFragment;

UCLASS(BlueprintType, Blueprintable)
class SANDBOXINVENTORY_API USBItemInstance : public UObject
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

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const USBItemFragment* FindFragmentByClass(TSubclassOf<USBItemFragment> FragmentClass) const;
};
