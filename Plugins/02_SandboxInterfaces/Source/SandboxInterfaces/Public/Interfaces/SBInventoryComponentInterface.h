// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SBInventoryComponentInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBInventoryComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contrato de notificacao de alteracao em item.
 *
 * Existe para que Extensoes de Gameplay irmas (ex.: 06_SandboxCombat) avisem o inventario de
 * que um item mudou — para que a replicacao seja marcada — sem referenciar o
 * USBInventoryComponent concreto, que pertence a 08_SandboxInventory.
 *
 * Substitui o par FindObject<UClass> por caminho textual + ProcessEvent sobre UFunction
 * encontrada por nome, com struct de parametros montada a mao.
 *
 * O parametro e UObject* porque USBItemInstance pertence a 08_SandboxInventory e nao pode
 * ser visto daqui. O nome difere do metodo concreto (MarkItemInstanceUpdated) de proposito:
 * assinaturas diferentes com o mesmo nome colidem no sistema de reflexao do UHT, e a
 * alternativa seria remover a UFUNCTION tipada do concreto, empobrecendo a API de Blueprint.
 */
class SANDBOXINTERFACES_API ISBInventoryComponentInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	void NotifyItemInstanceUpdated(UObject* ItemInstance);

	/**
	 * Devolve o conteudo atual ja em texto de exibicao ("Sucata x8").
	 *
	 * Existe pelo mesmo motivo do metodo acima, na direcao contraria: a UI (09_SandboxUI)
	 * precisa mostrar o inventario e nao pode ver USBItemInstance nem USBItemDefinition, que
	 * pertencem a 08_SandboxInventory. O que atravessa a fronteira e o texto pronto, nao o item.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	void GetInventoryDisplayLines(TArray<FText>& OutLines);
};
