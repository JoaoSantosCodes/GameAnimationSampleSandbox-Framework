#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SBCombatantInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBCombatantInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBCombatantInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Combat")
	bool IsTargetHostile(AActor* PotentialTarget) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Combat")
	FVector GetAttackSocketLocation(FName SocketName) const;
};
