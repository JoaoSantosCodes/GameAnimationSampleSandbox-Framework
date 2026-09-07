// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "SBAttributeComponentInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBAttributeComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contrato de leitura de atributos.
 *
 * Permite que módulos de Fundação (ex.: 04_SandboxCore) consultem atributos sem referenciar
 * o USBAttributeComponent concreto, que pertence a 05_SandboxCharacter — o que violaria o
 * Princípio 7 (Zero Dependências Circulares), já que SandboxCharacter depende de SandboxCore.
 *
 * Substitui o padrão anterior de resolver a classe por nome textual
 * (`FindObject<UClass>("/Script/SandboxCharacter.SBAttributeComponent")`) seguido de
 * `ProcessEvent` sobre a UFunction — frágil por não falhar em tempo de compilação se a
 * classe for renomeada ou movida.
 */
class SANDBOXINTERFACES_API ISBAttributeComponentInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Attributes")
	float GetAttributeValue(FGameplayTag AttributeTag) const;

	/**
	 * Teto do atributo, que vive em FSBAttribute::MaxValue e nao tem tag propria — existem
	 * Attribute.MaxHealth e Attribute.MaxWeight, mas nao equivalentes para estamina ou mana.
	 *
	 * Sem isto, um consumidor que so pode ver este contrato (ex.: 09_SandboxUI, que nao
	 * depende de 05_SandboxCharacter) consegue ler o valor corrente mas nao tem como formar
	 * uma proporcao, e fica obrigado a receber o teto empurrado por evento.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Attributes")
	float GetAttributeMaxValue(FGameplayTag AttributeTag) const;
};
