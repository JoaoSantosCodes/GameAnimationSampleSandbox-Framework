#pragma once

#include "CoreMinimal.h"
#include "DataAssets/SBPrimaryDataAsset.h"
#include "GameplayTagContainer.h"
#include "SBPawnDataAsset.generated.h"

class USBComponentSetDataAsset;
class USBAbilitySetDataAsset;

UCLASS(BlueprintType)
class SANDBOXASSETS_API USBPawnDataAsset : public USBPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<USkeletalMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSubclassOf<UAnimInstance> AnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UObject> InputConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USBComponentSetDataAsset> ComponentSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<USBAbilitySetDataAsset> AbilitySet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDLayoutClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer DefaultTags;
};
