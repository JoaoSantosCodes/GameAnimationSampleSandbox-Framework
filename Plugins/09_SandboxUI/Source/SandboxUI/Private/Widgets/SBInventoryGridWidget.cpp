// Copyright 2026 João Santos. All Rights Reserved.
#include "Widgets/SBInventoryGridWidget.h"
#include "Widgets/SBInventorySlotWidget.h"
#include "Subsystems/SBEventPayloads.h"
#include "Interfaces/SBInventoryComponentInterface.h"
#include "Blueprint/WidgetTree.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "SBGameplayTags.h"
#include "Utilities/SBLogCategories.h"

USBInventoryGridWidget::USBInventoryGridWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EmptyInventoryText = NSLOCTEXT("Sandbox", "InventoryEmpty", "Inventario vazio");
	InventorySummary = EmptyInventoryText;
	SlotWidgetClass = USBInventorySlotWidget::StaticClass();
	Columns = 4;
}

void USBInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FSBBlueprintEventDelegate SlotDelegate;
	SlotDelegate.BindDynamic(this, &USBInventoryGridWidget::OnSlotUpdated);
	SubscribeToEvent(FSBGameplayTags::Get().Event_Inventory_SlotUpdated, SlotDelegate);

	// Abrir o painel ja mostrando o que existe; sem isto, so apareceria algo depois de o
	// jogador mexer no inventario.
	RefreshInventorySummary();
}

UUniformGridPanel* USBInventoryGridWidget::ResolveSlotGrid()
{
	if (SlotGrid)
	{
		return SlotGrid;
	}

	// O Widget Blueprint nao precisa trazer a grade: se nao houver uma chamada SlotGrid, ela e
	// construida aqui e pendurada na raiz. E o que faz o pacote funcionar sem autoria de UMG.
	if (!WidgetTree)
	{
		return nullptr;
	}

	UPanelWidget* Raiz = Cast<UPanelWidget>(WidgetTree->RootWidget);
	if (!Raiz)
	{
		return nullptr;
	}

	SlotGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("SlotGrid"));
	Raiz->AddChild(SlotGrid);
	return SlotGrid;
}

void USBInventoryGridWidget::RefreshInventorySummary()
{
	TArray<FSBInventoryDisplayEntry> Entradas;

	if (const APawn* Pawn = GetOwningPlayerPawn())
	{
		// O componente concreto vive em 08_SandboxInventory e nao pode ser visto daqui; a busca
		// e pelo contrato, que e o mesmo motivo de ISBInventoryComponentInterface existir.
		for (UActorComponent* Component : Pawn->GetComponents())
		{
			if (!Component || !Component->GetClass()->ImplementsInterface(USBInventoryComponentInterface::StaticClass()))
			{
				continue;
			}

			ISBInventoryComponentInterface::Execute_GetInventoryDisplayEntries(Component, Entradas);
			break;
		}
	}

	// Resumo em texto continua existindo: e o que aparece com o inventario vazio, e serve a
	// paineis que preferem uma linha so a uma grade.
	if (Entradas.Num() == 0)
	{
		InventorySummary = EmptyInventoryText;
	}
	else
	{
		TArray<FText> Linhas;
		Linhas.Reserve(Entradas.Num());
		for (const FSBInventoryDisplayEntry& Entrada : Entradas)
		{
			Linhas.Add(Entrada.StackCount > 1
				? FText::Format(NSLOCTEXT("Sandbox", "InventoryLineStack", "{0} x{1}"), Entrada.Name, FText::AsNumber(Entrada.StackCount))
				: Entrada.Name);
		}
		InventorySummary = FText::Join(FText::FromString(TEXT("\n")), Linhas);
	}

	if (ContentsText)
	{
		// Com grade montada, o texto vira apenas o aviso de vazio — repetir a lista ao lado dos
		// slots seria a mesma informacao duas vezes na tela.
		ContentsText->SetText(Entradas.Num() == 0 ? InventorySummary : FText::GetEmpty());
	}

	RebuildSlots(Entradas);
}

void USBInventoryGridWidget::RebuildSlots(const TArray<FSBInventoryDisplayEntry>& Entradas)
{
	UUniformGridPanel* Grade = ResolveSlotGrid();
	if (!Grade || !WidgetTree)
	{
		return;
	}

	Grade->ClearChildren();

	UE_LOG(LogSandboxUI, Log, TEXT("Grade de inventario: %d slot(s)"), Entradas.Num());

	const int32 Colunas = FMath::Max(1, Columns);
	const TSubclassOf<USBInventorySlotWidget> Classe =
		SlotWidgetClass ? SlotWidgetClass : TSubclassOf<USBInventorySlotWidget>(USBInventorySlotWidget::StaticClass());

	for (int32 i = 0; i < Entradas.Num(); ++i)
	{
		USBInventorySlotWidget* SlotWidget = CreateWidget<USBInventorySlotWidget>(GetOwningPlayer(), Classe);
		if (!SlotWidget)
		{
			continue;
		}

		SlotWidget->SetEntry(Entradas[i]);

		if (UUniformGridSlot* Celula = Grade->AddChildToUniformGrid(SlotWidget, i / Colunas, i % Colunas))
		{
			Celula->SetHorizontalAlignment(HAlign_Fill);
			Celula->SetVerticalAlignment(VAlign_Fill);
		}
	}
}

void USBInventoryGridWidget::OnSlotUpdated(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;

	USBInventoryEventPayload* InvPayload = Cast<USBInventoryEventPayload>(Payload);
	if (!InvPayload) return;

	if (InvPayload->TargetPawn != GetOwningPlayerPawn()) return;

	RefreshInventorySummary();

	BP_OnSlotUpdated(InvPayload->ItemInstance);
}
