// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Engine/Texture2D.h"
#include "SBInventoryComponentInterface.generated.h"

/**
 * Uma linha do inventario pronta para desenhar.
 *
 * Existe para que a UI (09_SandboxUI) monte a grade sem enxergar USBItemInstance nem
 * USBItemDefinition, que pertencem a 08_SandboxInventory: o que atravessa a fronteira e o
 * dado de exibicao, nao o item.
 */
USTRUCT(BlueprintType)
struct FSBInventoryDisplayEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FText Name;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 StackCount = 1;

	/** Vazio quando o item nao tem arte; nesse caso o slot cai na cor de raridade. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FLinearColor RarityColor = FLinearColor(0.55f, 0.55f, 0.55f, 1.0f);
};

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
	 * Devolve o conteudo atual em forma de exibicao: nome, quantidade, icone e cor.
	 *
	 * Mesma razao do metodo acima, na direcao contraria — a UI precisa mostrar o inventario e
	 * nao pode ver as classes de item.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	void GetInventoryDisplayEntries(TArray<FSBInventoryDisplayEntry>& OutEntries);
};
