#include "Subsystems/SBSandboxRuleSubsystem.h"
#include "Interfaces/SBCharacterInterface.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "Interfaces/SBAttributeComponentInterface.h"

USBSandboxRuleSubsystem::USBSandboxRuleSubsystem()
{
}

bool USBSandboxRuleSubsystem::EvaluateRule(const FSBRule& Rule, AActor* TargetActor) const
{
	if (Rule.Conditions.Num() == 0)
	{
		return true;
	}

	if (Rule.bRequireAll)
	{
		// Lógica AND: todas as condições devem ser verdadeiras
		for (const FSBRuleCondition& Condition : Rule.Conditions)
		{
			if (!EvaluateCondition(Condition, TargetActor))
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		// Lógica OR: pelo menos uma deve ser verdadeira
		for (const FSBRuleCondition& Condition : Rule.Conditions)
		{
			if (EvaluateCondition(Condition, TargetActor))
			{
				return true;
			}
		}
		return false;
	}
}

bool USBSandboxRuleSubsystem::EvaluateCondition(const FSBRuleCondition& Condition, AActor* TargetActor) const
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	// 1. Caso de verificação direta de posse de Tags (HasTag / DoesNotHaveTag)
	if (Condition.Operator == ESBRuleOperator::HasTag || Condition.Operator == ESBRuleOperator::DoesNotHaveTag)
	{
		UActorComponent* StateComp = nullptr;
		if (TargetActor->Implements<USBCharacterInterface>())
		{
			StateComp = ISBCharacterInterface::Execute_GetStateComponent(TargetActor);
		}

		// Resolucao por contrato primeiro. ISBCharacterInterface::GetStateComponent e um
		// acessor de conveniencia que, em ASBCharacter, retorna nulo: o UHT nao registrou
		// o override e a chamada cai na implementacao default da interface. O contrato
		// ISBStateComponentInterface resolve corretamente e e mais desacoplado (Principio 4).
		if (!StateComp)
		{
			StateComp = TargetActor->FindComponentByInterface(USBStateComponentInterface::StaticClass());
		}

		bool bHasTag = false;
		if (StateComp && StateComp->Implements<USBStateComponentInterface>())
		{
			bHasTag = ISBStateComponentInterface::Execute_HasTag(StateComp, Condition.TagValue);
		}

		if (Condition.Operator == ESBRuleOperator::HasTag)
		{
			return bHasTag;
		}
		else
		{
			return !bHasTag;
		}
	}

	// 2. Caso de comparações matemáticas / numéricas (Atributos ou Tags de Estado)
	float CurrentValue = 0.0f;
	bool bValueResolved = false;

	FString TagName = Condition.ConditionTag.ToString();
	if (TagName.StartsWith(TEXT("Attribute.")))
	{
		UActorComponent* AttrComp = nullptr;
		if (TargetActor->Implements<USBCharacterInterface>())
		{
			AttrComp = ISBCharacterInterface::Execute_GetAttributeComponent(TargetActor);
		}

			// Mesma razao do bloco de estado: o acessor de conveniencia do personagem nao e
			// confiavel, entao o contrato do componente vem primeiro.
		if (!AttrComp)
		{
			AttrComp = TargetActor->FindComponentByInterface(USBAttributeComponentInterface::StaticClass());
		}

		if (AttrComp && AttrComp->Implements<USBAttributeComponentInterface>())
		{
			CurrentValue = ISBAttributeComponentInterface::Execute_GetAttributeValue(AttrComp, Condition.ConditionTag);
			bValueResolved = true;
		}
	}
	else
	{
		// Se for uma tag de estado genérica, tratamos posse como 1.0 e ausência como 0.0
		UActorComponent* StateComp = nullptr;
		if (TargetActor->Implements<USBCharacterInterface>())
		{
			StateComp = ISBCharacterInterface::Execute_GetStateComponent(TargetActor);
		}

		// Resolucao por contrato primeiro. ISBCharacterInterface::GetStateComponent e um
		// acessor de conveniencia que, em ASBCharacter, retorna nulo: o UHT nao registrou
		// o override e a chamada cai na implementacao default da interface. O contrato
		// ISBStateComponentInterface resolve corretamente e e mais desacoplado (Principio 4).
		if (!StateComp)
		{
			StateComp = TargetActor->FindComponentByInterface(USBStateComponentInterface::StaticClass());
		}

		if (StateComp && StateComp->Implements<USBStateComponentInterface>())
		{
			bool bHasTag = ISBStateComponentInterface::Execute_HasTag(StateComp, Condition.ConditionTag);
			CurrentValue = bHasTag ? 1.0f : 0.0f;
			bValueResolved = true;
		}
	}

	if (!bValueResolved)
	{
		return false;
	}

	// Executa a comparação matemática correspondente
	switch (Condition.Operator)
	{
	case ESBRuleOperator::Equal:
		return FMath::IsNearlyEqual(CurrentValue, Condition.NumericValue);
	case ESBRuleOperator::NotEqual:
		return !FMath::IsNearlyEqual(CurrentValue, Condition.NumericValue);
	case ESBRuleOperator::LessThan:
		return CurrentValue < Condition.NumericValue;
	case ESBRuleOperator::LessThanOrEqual:
		return CurrentValue <= Condition.NumericValue;
	case ESBRuleOperator::GreaterThan:
		return CurrentValue > Condition.NumericValue;
	case ESBRuleOperator::GreaterThanOrEqual:
		return CurrentValue >= Condition.NumericValue;
	default:
		return false;
	}
}
