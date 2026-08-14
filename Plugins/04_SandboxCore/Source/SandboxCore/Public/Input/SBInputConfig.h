#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SBInputConfig.generated.h"

class UInputAction;

USTRUCT(BlueprintType)
struct FSBInputActionMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InputAction = nullptr;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<FSBInputActionMapping> InputActions;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Input")
	const UInputAction* FindInputActionForTag(const FGameplayTag& InputTag) const;
};
