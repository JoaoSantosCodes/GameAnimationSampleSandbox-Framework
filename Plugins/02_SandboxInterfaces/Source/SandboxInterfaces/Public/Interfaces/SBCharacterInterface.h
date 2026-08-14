#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SBCharacterInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBCharacterInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Character")
	UObject* GetPawnData() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Character")
	UActorComponent* GetAttributeComponent() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Character")
	UActorComponent* GetStateComponent() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Character")
	UActorComponent* GetAbilityComponent() const;
};
