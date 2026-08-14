#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "SBStateComponentInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBStateComponentInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBStateComponentInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
	bool HasTag(FGameplayTag StateTag) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
	bool HasAny(FGameplayTagContainer TagsContainer) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "State")
	bool HasAll(FGameplayTagContainer TagsContainer) const;
};
