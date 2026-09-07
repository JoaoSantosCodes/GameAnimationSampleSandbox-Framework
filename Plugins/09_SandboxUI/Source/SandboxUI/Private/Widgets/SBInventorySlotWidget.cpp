// Copyright 2026 João Santos. All Rights Reserved.
#include "Widgets/SBInventorySlotWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

TSharedRef<SWidget> USBInventorySlotWidget::RebuildWidget()
{
	// Arvore montada uma vez, no primeiro Rebuild; nas reconstrucoes seguintes o WidgetTree ja
	// tem raiz e reaproveita o que existe.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		USizeBox* Caixa = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SlotBox"));
		Caixa->SetWidthOverride(96.0f);
		Caixa->SetHeightOverride(96.0f);
		WidgetTree->RootWidget = Caixa;

		UOverlay* Pilha = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("SlotOverlay"));
		Caixa->AddChild(Pilha);

		IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("IconImage"));
		if (UOverlaySlot* SlotIcone = Cast<UOverlaySlot>(Pilha->AddChild(IconImage)))
		{
			SlotIcone->SetHorizontalAlignment(HAlign_Fill);
			SlotIcone->SetVerticalAlignment(VAlign_Fill);
		}

		NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NameText"));
		NameText->SetJustification(ETextJustify::Center);
		if (UOverlaySlot* SlotNome = Cast<UOverlaySlot>(Pilha->AddChild(NameText)))
		{
			SlotNome->SetHorizontalAlignment(HAlign_Center);
			SlotNome->SetVerticalAlignment(VAlign_Bottom);
		}

		CountText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CountText"));
		if (UOverlaySlot* SlotQtd = Cast<UOverlaySlot>(Pilha->AddChild(CountText)))
		{
			SlotQtd->SetHorizontalAlignment(HAlign_Right);
			SlotQtd->SetVerticalAlignment(VAlign_Top);
		}
	}

	return Super::RebuildWidget();
}

void USBInventorySlotWidget::SetEntry(const FSBInventoryDisplayEntry& Entry)
{
	if (NameText)
	{
		NameText->SetText(Entry.Name);
	}

	if (CountText)
	{
		// Pilha de um so nao ganha "x1" pendurado: numero repetido em todo slot vira ruido.
		CountText->SetText(Entry.StackCount > 1
			? FText::AsNumber(Entry.StackCount)
			: FText::GetEmpty());
	}

	if (!IconImage)
	{
		return;
	}

	// Item sem arte nao pode virar buraco na grade: cai no quadrado com a cor da raridade.
	if (UTexture2D* Textura = Entry.Icon.LoadSynchronous())
	{
		IconImage->SetBrushFromTexture(Textura, /*bMatchSize=*/false);
		IconImage->SetColorAndOpacity(FLinearColor::White);
	}
	else
	{
		IconImage->SetBrushFromTexture(nullptr, false);
		IconImage->SetColorAndOpacity(Entry.RarityColor);
	}
}
