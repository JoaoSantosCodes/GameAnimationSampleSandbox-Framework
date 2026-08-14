#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBMovementConfigDataAsset.generated.h"

class USBMovementBehavior;
class USBMovementBehaviorDefinition;

USTRUCT(BlueprintType)
struct FSBMovementBehaviorConfigEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	TSubclassOf<USBMovementBehavior> BehaviorClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<USBMovementBehaviorDefinition> DefinitionAsset = nullptr;
};

UCLASS(BlueprintType)
class SANDBOXCHARACTER_API USBMovementConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	TArray<FSBMovementBehaviorConfigEntry> ConfiguredBehaviors;
};
