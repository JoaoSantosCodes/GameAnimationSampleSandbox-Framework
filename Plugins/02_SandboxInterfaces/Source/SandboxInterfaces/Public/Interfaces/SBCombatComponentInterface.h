#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "SBCombatComponentInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBCombatComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contrato de consulta ao estado de armas de um combatente.
 *
 * Existe para que Extensoes de Gameplay irmas (ex.: 08_SandboxInventory) perguntem sobre a
 * arma ativa sem referenciar o USBCombatComponent concreto, que pertence a 06_SandboxCombat.
 * A SFPS proibe dependencia entre plugins irmaos, e a restricao esta correta — o contorno
 * anterior e que violava o Principio 4.
 *
 * Substitui tres niveis de reflexao encadeados no consumidor: classe por caminho textual,
 * UFunction por nome (duas vezes) e FProperty por nome. Nenhum deles falha em tempo de
 * compilacao se algo for renomeado; todos falham em silencio em runtime.
 *
 * A pergunta e exposta ja resolvida ("tem arma ativa com esta tag?") em vez de devolver a
 * lista de armas: manter a semantica de arma dentro de 06_SandboxCombat evita que o chamador
 * volte a inspecionar objetos de outro plugin para interpretar a resposta.
 */
class SANDBOXINTERFACES_API ISBCombatComponentInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool HasActiveWeaponWithTag(FGameplayTag WeaponTag) const;
};
