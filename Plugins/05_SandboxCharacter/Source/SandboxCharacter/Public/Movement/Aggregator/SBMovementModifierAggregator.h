#pragma once

#include "CoreMinimal.h"
#include "Types/SBCommonTypes.h"
#include "SBMovementModifierAggregator.generated.h"

UCLASS(BlueprintType)
class SANDBOXCHARACTER_API USBMovementModifierAggregator : public USBModifierAggregator
{
	GENERATED_BODY()

public:
	USBMovementModifierAggregator();

	// Adiciona/atualiza os modificadores de uma fonte (ex: um Behavior)
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Movement|Aggregator")
	void SetModifiersForSource(FGameplayTag SourceTag, const TArray<FSBModifierEntry>& Entries);

	// Remove todos os modificadores associados a uma fonte (ex: ao sair de um Behavior)
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Movement|Aggregator")
	void ClearModifiersForSource(FGameplayTag SourceTag);

	// Recalcula o valor final aplicando a pilha de modificadores de forma determinística
	UFUNCTION(BlueprintPure, Category = "Sandbox|Movement|Aggregator")
	float CalculateFinalValue(float BaseValue) const;

protected:
	// Lista acumulada de modificadores físicos ativos
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Sandbox|Movement|Aggregator")
	TArray<FSBModifierEntry> ActiveModifiers;
};
