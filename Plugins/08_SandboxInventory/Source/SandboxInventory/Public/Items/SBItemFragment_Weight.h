#pragma once

#include "CoreMinimal.h"
#include "Items/SBItemFragment.h"
#include "SBItemFragment_Weight.generated.h"

UCLASS(BlueprintType, meta = (DisplayName = "Weight Fragment"))
class SANDBOXINVENTORY_API USBItemFragment_Weight : public USBItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weight", meta = (ClampMin = "0.0"))
	float Weight = 0.1f;
};
