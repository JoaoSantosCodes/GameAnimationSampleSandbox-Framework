#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SBComponentFactory.generated.h"

class USBComponentSetDataAsset;

UCLASS(BlueprintType)
class SANDBOXCORE_API USBComponentFactory : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sandbox|ComponentFactory")
	static void InitializeComponentsFromSet(AActor* TargetActor, USBComponentSetDataAsset* ComponentSet);
};
