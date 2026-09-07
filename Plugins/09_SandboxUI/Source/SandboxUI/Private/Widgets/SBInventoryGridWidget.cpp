// Copyright 2026 João Santos. All Rights Reserved.
#include "Widgets/SBInventoryGridWidget.h"
#include "Subsystems/SBEventPayloads.h"
#include "Interfaces/SBInventoryComponentInterface.h"
#include "GameFramework/Pawn.h"
#include "SBGameplayTags.h"

USBInventoryGridWidget::USBInventoryGridWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EmptyInventoryText = NSLOCTEXT("Sandbox", "InventoryEmpty", "Inventario vazio");
	InventorySummary = EmptyInventoryText;
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

void USBInventoryGridWidget::RefreshInventorySummary()
{
	InventorySummary = EmptyInventoryText;

	const APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn)
	{
		return;
	}

	// O componente concreto vive em 08_SandboxInventory e nao pode ser visto daqui; a busca e
	// pelo contrato, que e o mesmo motivo de ISBInventoryComponentInterface existir.
	for (UActorComponent* Component : Pawn->GetComponents())
	{
		if (!Component || !Component->GetClass()->ImplementsInterface(USBInventoryComponentInterface::StaticClass()))
		{
			continue;
		}

		TArray<FText> Linhas;
		ISBInventoryComponentInterface::Execute_GetInventoryDisplayLines(Component, Linhas);
		if (Linhas.Num() > 0)
		{
			InventorySummary = FText::Join(FText::FromString(TEXT("\n")), Linhas);
		}
		break;
	}

	// Escrever direto no Text Block, em vez de property binding: binding e reavaliado a cada
	// frame pela engine, e a ferramenta que os cria gerava grafo malformado.
	if (ContentsText)
	{
		ContentsText->SetText(InventorySummary);
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
