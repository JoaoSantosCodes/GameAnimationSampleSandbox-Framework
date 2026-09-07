// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SBItemDurabilityInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBItemDurabilityInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBItemDurabilityInterface
{
	GENERATED_BODY()

public:
	// Obtém a durabilidade atual
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	float GetDurability() const;

	// Define a durabilidade atual (Servidor apenas)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	void SetDurability(float NewDurability);

	// Consome durabilidade (Servidor apenas)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Durability")
	void ConsumeDurability(float Amount);
};
