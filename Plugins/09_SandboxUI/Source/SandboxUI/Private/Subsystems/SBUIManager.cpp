#include "Subsystems/SBUIManager.h"
#include "Utilities/SBLogCategories.h"
#include "Blueprint/UserWidget.h"

USBUIManager::USBUIManager()
{
}

USBUserWidget* USBUIManager::PushWidget(TSubclassOf<USBUserWidget> WidgetClass, ESBUILayer Layer)
{
	if (!WidgetClass) return nullptr;

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer) return nullptr;

	APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
	if (!PC) return nullptr;

	USBUserWidget* NewWidget = CreateWidget<USBUserWidget>(PC, WidgetClass);
	if (NewWidget)
	{
		// Set Z-Order based on Layer to enforce stack ordering
		int32 ZOrder = static_cast<int32>(Layer);
		NewWidget->AddToViewport(ZOrder);

		LayerStacks.FindOrAdd(Layer).Stack.Push(NewWidget);
		NewWidget->OnPushed();

		UE_LOG(LogSandboxUI, Log, TEXT("Pushed widget %s onto layer %d (Z-Order: %d)"), *WidgetClass->GetName(), static_cast<int32>(Layer), ZOrder);
		return NewWidget;
	}

	return nullptr;
}

void USBUIManager::PopWidget(ESBUILayer Layer)
{
	FWidgetStackWrapper* StackWrapper = LayerStacks.Find(Layer);
	if (StackWrapper && StackWrapper->Stack.Num() > 0)
	{
		USBUserWidget* TopWidget = StackWrapper->Stack.Pop();
		if (TopWidget)
		{
			TopWidget->OnPopped();
			TopWidget->RemoveFromParent();
			UE_LOG(LogSandboxUI, Log, TEXT("Popped top widget from layer %d"), static_cast<int32>(Layer));
		}
	}
}

void USBUIManager::ClearLayer(ESBUILayer Layer)
{
	FWidgetStackWrapper* StackWrapper = LayerStacks.Find(Layer);
	if (StackWrapper)
	{
		while (StackWrapper->Stack.Num() > 0)
		{
			USBUserWidget* Widget = StackWrapper->Stack.Pop();
			if (Widget)
			{
				Widget->OnPopped();
				Widget->RemoveFromParent();
			}
		}
		UE_LOG(LogSandboxUI, Log, TEXT("Cleared all widgets from layer %d"), static_cast<int32>(Layer));
	}
}
