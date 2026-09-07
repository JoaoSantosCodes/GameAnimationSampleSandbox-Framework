// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBUserWidget.h"
#include "Components/TextBlock.h"
#include "Interfaces/SBInventoryComponentInterface.h"
#include "SBInventoryGridWidget.generated.h"

class UUniformGridPanel;
class USBInventorySlotWidget;

UCLASS(Abstract, Blueprintable)
class SANDBOXUI_API USBInventoryGridWidget : public USBUserWidget
{
	GENERATED_BODY()

public:
	USBInventoryGridWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Conteudo do inventario do pawn dono em uma linha por item ("Sucata x8").
	 *
	 * Continua existindo depois da grade porque e o que aparece com o inventario vazio, e
	 * atende a paineis que preferem uma linha unica a um grid de slots.
	 */
	UFUNCTION(BlueprintPure, Category = "Sandbox|UI")
	FText GetInventorySummary() const { return InventorySummary; }

	/** Reconsulta o inventario, remonta os slots e atualiza o resumo. */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|UI")
	void RefreshInventorySummary();

	/** Texto exibido quando o pawn nao tem inventario ou ele esta vazio. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sandbox|UI")
	FText EmptyInventoryText;

	/** Widget usado em cada slot. Trocavel por heranca, sem mexer nesta classe. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sandbox|UI")
	TSubclassOf<USBInventorySlotWidget> SlotWidgetClass;

	/** Colunas da grade. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sandbox|UI", meta = (ClampMin = "1"))
	int32 Columns;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnSlotUpdated(FGameplayTag EventTag, UObject* Payload);

	UFUNCTION(BlueprintImplementableEvent, Category = "Sandbox|UI")
	void BP_OnSlotUpdated(UObject* ItemInstance);

	/** Devolve a grade do Blueprint ou constroi uma na raiz quando ele nao traz nenhuma. */
	UUniformGridPanel* ResolveSlotGrid();

	void RebuildSlots(const TArray<FSBInventoryDisplayEntry>& Entradas);

	UPROPERTY(BlueprintReadOnly, Category = "Sandbox|UI")
	FText InventorySummary;

	/**
	 * Text Block que recebe o resumo, preenchido por nome pelo Widget Blueprint.
	 *
	 * Opcional de proposito: um painel que queira desenhar os itens de outro jeito continua
	 * valido. Escrever aqui direto evita property binding, que a engine reavalia a cada frame.
	 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Sandbox|UI")
	TObjectPtr<UTextBlock> ContentsText;

	/** Grade de slots. Opcional: sem ela, uma e construida em tempo de execucao. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Sandbox|UI")
	TObjectPtr<UUniformGridPanel> SlotGrid;
};
