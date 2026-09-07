// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/SBInventoryComponentInterface.h"
#include "SBInventorySlotWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * Um slot da grade de inventario: icone, nome e quantidade.
 *
 * Monta a propria arvore de widgets em C++ (RebuildWidget) de proposito. Assim o pacote
 * funciona sem que o comprador precise criar Widget Blueprint algum — e quem quiser outro
 * visual ainda pode herdar e substituir.
 */
UCLASS()
class SANDBOXUI_API USBInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Preenche o slot. Icone vazio cai no quadrado com a cor de raridade. */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|UI")
	void SetEntry(const FSBInventoryDisplayEntry& Entry);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(Transient)
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CountText;
};
