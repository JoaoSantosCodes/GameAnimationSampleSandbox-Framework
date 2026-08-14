#pragma once

#include "CoreMinimal.h"
#include "DataAssets/SBPrimaryDataAsset.h"
#include "GameplayTagContainer.h"
#include "SBAbilitySetDataAsset.generated.h"

class USBGameplayBehavior;
class USBGameplayBehaviorDefinition;

USTRUCT(BlueprintType)
struct FSBAbilitySetEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<USBGameplayBehavior> AbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<USBGameplayBehaviorDefinition> Definition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FGameplayTag InputTag;
};

UCLASS(BlueprintType)
class SANDBOXASSETS_API USBAbilitySetDataAsset : public USBPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<FSBAbilitySetEntry> Abilities;
};
