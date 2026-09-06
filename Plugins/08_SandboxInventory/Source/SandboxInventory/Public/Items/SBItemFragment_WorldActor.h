#pragma once

#include "CoreMinimal.h"
#include "Items/SBItemFragment.h"
#include "SBItemFragment_WorldActor.generated.h"

/**
 * Fragmento de item que define a classe de ator que o representa quando spawnado fisicamente no mundo.
 */
UCLASS(BlueprintType)
class SANDBOXINVENTORY_API USBItemFragment_WorldActor : public USBItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement")
	TSubclassOf<AActor> WorldActorClass = nullptr;
};
