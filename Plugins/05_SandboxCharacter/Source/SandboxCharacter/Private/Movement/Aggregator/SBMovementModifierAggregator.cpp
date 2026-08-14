#include "Movement/Aggregator/SBMovementModifierAggregator.h"

USBMovementModifierAggregator::USBMovementModifierAggregator()
{
}

void USBMovementModifierAggregator::SetModifiersForSource(FGameplayTag SourceTag, const TArray<FSBModifierEntry>& Entries)
{
	if (!SourceTag.IsValid()) return;

	// 1. Limpa os modificadores antigos desta fonte para evitar duplicações
	ClearModifiersForSource(SourceTag);

	// 2. Insere os novos modificadores no acumulador
	ActiveModifiers.Append(Entries);
}

void USBMovementModifierAggregator::ClearModifiersForSource(FGameplayTag SourceTag)
{
	if (!SourceTag.IsValid()) return;

	// Remove todos os registros cujo SourceTag coincide
	ActiveModifiers.RemoveAll([SourceTag](const FSBModifierEntry& Entry)
	{
		return Entry.SourceTag == SourceTag;
	});
}

float USBMovementModifierAggregator::CalculateFinalValue(float BaseValue) const
{
	if (ActiveModifiers.Num() == 0)
	{
		return BaseValue;
	}

	// 1. Cria uma cópia local para ordenação
	TArray<FSBModifierEntry> SortedModifiers = ActiveModifiers;

	// 2. Executa StableSort (ordenador estável que mantém a ordem de inserção determinística para prioridades idênticas)
	SortedModifiers.StableSort([](const FSBModifierEntry& A, const FSBModifierEntry& B)
	{
		return A.Priority < B.Priority; // Ascendente: prioridades maiores rodam depois (overrides por último)
	});

	// 3. Resolve os modificadores de forma sequencial determinística
	float Value = BaseValue;
	for (const FSBModifierEntry& Entry : SortedModifiers)
	{
		switch (Entry.Operation)
		{
			case ESBModifierOperation::Additive:
				Value += Entry.Value;
				break;

			case ESBModifierOperation::Multiply:
				Value *= Entry.Value;
				break;

			case ESBModifierOperation::Override:
				Value = Entry.Value;
				break;

			default:
				break;
		}
	}

	return Value;
}
