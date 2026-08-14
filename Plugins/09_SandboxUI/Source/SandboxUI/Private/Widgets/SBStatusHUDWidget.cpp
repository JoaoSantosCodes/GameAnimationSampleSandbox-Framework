#include "Widgets/SBStatusHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Subsystems/SBEventPayloads.h"

USBStatusHUDWidget::USBStatusHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USBStatusHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FSBBlueprintEventDelegate Delegate;
	Delegate.BindDynamic(this, &USBStatusHUDWidget::OnAttributeChanged);
	SubscribeToEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Attribute.Changed")), Delegate);
}

void USBStatusHUDWidget::OnAttributeChanged(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;

	USBAttributeChangedPayload* AttrPayload = Cast<USBAttributeChangedPayload>(Payload);
	if (!AttrPayload) return;

	if (AttrPayload->TargetPawn != GetOwningPlayerPawn()) return;

	if (AttrPayload->AttributeTag == FGameplayTag::RequestGameplayTag(TEXT("Attribute.Character.Health")))
	{
		if (PB_Health)
		{
			float Pct = (AttrPayload->MaxValue > 0.0f) ? (AttrPayload->CurrentValue / AttrPayload->MaxValue) : 0.0f;
			PB_Health->SetPercent(FMath::Clamp(Pct, 0.0f, 1.0f));
		}
	}
	else if (AttrPayload->AttributeTag == FGameplayTag::RequestGameplayTag(TEXT("Attribute.Mana")))
	{
		if (PB_Mana)
		{
			float Pct = (AttrPayload->MaxValue > 0.0f) ? (AttrPayload->CurrentValue / AttrPayload->MaxValue) : 0.0f;
			PB_Mana->SetPercent(FMath::Clamp(Pct, 0.0f, 1.0f));
		}
	}
}
