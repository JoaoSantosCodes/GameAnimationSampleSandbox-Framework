// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBUserWidget.h"
#include "Components/TextBlock.h"
#include "SBInventoryGridWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class SANDBOXUI_API USBInventoryGridWidget : public USBUserWidget
{
	GENERATED_BODY()

public:
	USBInventoryGridWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Conteudo do inventario do pawn dono, em uma linha por item ("Sucata x8").
	 *
	 * Um Text Block ligado a este metodo ja mostra o inventario sem nenhum no de Blueprint —
	 * antes disto, o widget so servia como classe-base e nao exibia nada sozinho.
	 */
	UFUNCTION(BlueprintPure, Category = "Sandbox|UI")
	FText GetInventorySummary() const { return InventorySummary; }

	/** Reconsulta o inventario e atualiza o resumo. Chamado ao abrir e a cada slot alterado. */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|UI")
	void RefreshInventorySummary();

	/** Texto exibido quando o pawn nao tem inventario ou ele esta vazio. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sandbox|UI")
	FText EmptyInventoryText;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnSlotUpdated(FGameplayTag EventTag, UObject* Payload);

	UFUNCTION(BlueprintImplementableEvent, Category = "Sandbox|UI")
	void BP_OnSlotUpdated(UObject* ItemInstance);

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
};
