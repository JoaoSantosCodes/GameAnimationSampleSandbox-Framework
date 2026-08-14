#include "Widgets/SBInteractionPromptWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Subsystems/SBEventPayloads.h"

USBInteractionPromptWidget::USBInteractionPromptWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USBInteractionPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Collapsed);

	FSBBlueprintEventDelegate AvailableDelegate;
	AvailableDelegate.BindDynamic(this, &USBInteractionPromptWidget::OnInteractionAvailable);
	SubscribeToEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Available")), AvailableDelegate);

	FSBBlueprintEventDelegate ClearedDelegate;
	ClearedDelegate.BindDynamic(this, &USBInteractionPromptWidget::OnInteractionCleared);
	SubscribeToEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Cleared")), ClearedDelegate);

	FSBBlueprintEventDelegate ProgressDelegate;
	ProgressDelegate.BindDynamic(this, &USBInteractionPromptWidget::OnInteractionProgress);
	SubscribeToEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Progress")), ProgressDelegate);
}

void USBInteractionPromptWidget::OnInteractionAvailable(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;

	USBInteractionAvailableEventPayload* AvailPayload = Cast<USBInteractionAvailableEventPayload>(Payload);
	if (!AvailPayload) return;

	if (AvailPayload->TargetPawn != GetOwningPlayerPawn()) return;

	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (TXT_Prompt)
	{
		TXT_Prompt->SetText(AvailPayload->PromptText);
	}

	if (PB_HoldProgress)
	{
		PB_HoldProgress->SetPercent(0.0f);
		PB_HoldProgress->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USBInteractionPromptWidget::OnInteractionCleared(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;

	USBPawnEventPayload* PawnPayload = Cast<USBPawnEventPayload>(Payload);
	if (!PawnPayload) return;

	if (PawnPayload->TargetPawn != GetOwningPlayerPawn()) return;

	SetVisibility(ESlateVisibility::Collapsed);
}

void USBInteractionPromptWidget::OnInteractionProgress(FGameplayTag EventTag, UObject* Payload)
{
	if (!Payload) return;

	USBInteractionProgressEventPayload* ProgPayload = Cast<USBInteractionProgressEventPayload>(Payload);
	if (!ProgPayload) return;

	if (ProgPayload->TargetPawn != GetOwningPlayerPawn()) return;

	if (PB_HoldProgress)
	{
		PB_HoldProgress->SetVisibility(ESlateVisibility::Visible);
		PB_HoldProgress->SetPercent(ProgPayload->ProgressPercent);
	}
}
