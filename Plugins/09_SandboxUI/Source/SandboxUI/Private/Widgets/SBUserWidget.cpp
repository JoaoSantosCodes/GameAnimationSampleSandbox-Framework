#include "Widgets/SBUserWidget.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

USBUserWidget::USBUserWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USBUserWidget::OnPushed_Implementation()
{
}

void USBUserWidget::OnPopped_Implementation()
{
}

void USBUserWidget::SubscribeToEvent(FGameplayTag EventTag, FSBBlueprintEventDelegate Delegate)
{
	if (!Delegate.IsBound()) return;

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USBEventSubsystem* EventSubsystem = GI->GetSubsystem<USBEventSubsystem>())
			{
				EventSubsystem->SubscribeToEvent(EventTag, ESBEventPriority::Medium, Delegate);

				FSBWidgetEventSubscription Sub;
				Sub.EventTag = EventTag;
				Sub.Delegate = Delegate;
				Subscriptions.Add(Sub);
			}
		}
	}
}

void USBUserWidget::UnsubscribeAllEvents()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USBEventSubsystem* EventSubsystem = GI->GetSubsystem<USBEventSubsystem>())
			{
				for (const FSBWidgetEventSubscription& Sub : Subscriptions)
				{
					EventSubsystem->UnsubscribeFromEvent(Sub.EventTag, Sub.Delegate);
				}
			}
		}
	}
	Subscriptions.Empty();
}

void USBUserWidget::NativeDestruct()
{
	UnsubscribeAllEvents();
	Super::NativeDestruct();
}

APawn* USBUserWidget::GetOwningPlayerPawn() const
{
#if WITH_EDITOR
	if (bMockOwningPawn)
	{
		return MockOwningPawn;
	}
#endif
	APlayerController* PC = GetOwningPlayer();
	return PC ? PC->GetPawn() : nullptr;
}
