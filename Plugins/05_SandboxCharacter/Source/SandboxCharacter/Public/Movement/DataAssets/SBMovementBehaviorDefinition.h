#pragma once

#include "CoreMinimal.h"
#include "Behaviors/SBGameplayBehaviorDefinition.h"
#include "Types/SBCommonTypes.h"
#include "SBMovementBehaviorDefinition.generated.h"

UCLASS(BlueprintType)
class SANDBOXCHARACTER_API USBMovementBehaviorDefinition : public USBGameplayBehaviorDefinition
{
	GENERATED_BODY()

public:
	// Se verdadeiro, a entrada funciona como Toggle. Se falso, funciona como Hold
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	bool bIsToggleActivation = false;

	// Modificadores físicos aplicados por este comportamento ao Aggregator
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Modifiers")
	TArray<FSBModifierEntry> MovementModifiers;

	// Taxa de consumo de Stamina por segundo (0 para sem custo)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior|Attributes")
	float StaminaCostPerSecond = 10.0f;
};
