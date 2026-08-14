#include "Widgets/SBInventoryGridWidget.h"
#include "Subsystems/SBEventPayloads.h"

USBInventoryGridWidget::USBInventoryGridWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USBInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FSBBlueprintEventDelegate SlotDelegate;
	SlotDelegate.BindDynamic(this, &USBInventoryGridWidget::OnSlotUpdated);
	SubscribeToEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.SlotUpdated")), SlotDelegate);
}

void USBInventoryGridWidget::OnSlotUpdated(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;

	USBInventoryEventPayload* InvPayload = Cast<USBInventoryEventPayload>(Payload);
	if (!InvPayload) return;

	if (InvPayload->TargetPawn != GetOwningPlayerPawn()) return;

	BP_OnSlotUpdated(InvPayload->ItemInstance);
}
