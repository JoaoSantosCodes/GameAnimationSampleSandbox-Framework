#pragma once

#include "CoreMinimal.h"
#include "DataAssets/SBPrimaryDataAsset.h"
#include "SBComponentSetDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FSBComponentSetEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	TSubclassOf<UActorComponent> ComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	TArray<TSubclassOf<UActorComponent>> Dependencies;
};

UCLASS(BlueprintType)
class SANDBOXASSETS_API USBComponentSetDataAsset : public USBPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<FSBComponentSetEntry> Components;
};
