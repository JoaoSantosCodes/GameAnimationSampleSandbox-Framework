#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBGameplayBehaviorDefinition.generated.h"

UCLASS(BlueprintType, Const)
class SANDBOXCOMMON_API USBGameplayBehaviorDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	FGameplayTag BehaviorTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	int32 StackPriority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	FGameplayTag ExclusivityGroup;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	FGameplayTagContainer BlockedTags;
};
