#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBItemDefinition.generated.h"

class USBItemFragment;

UCLASS(BlueprintType, Const)
class SANDBOXINVENTORY_API USBItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stacking")
	int32 MaxStackCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Fragments")
	TArray<TObjectPtr<USBItemFragment>> Fragments;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const USBItemFragment* FindFragmentByClass(TSubclassOf<USBItemFragment> FragmentClass) const;
};
