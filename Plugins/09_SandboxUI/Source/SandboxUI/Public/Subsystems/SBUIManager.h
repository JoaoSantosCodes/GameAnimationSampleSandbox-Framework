#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Widgets/SBUserWidget.h"
#include "SBUIManager.generated.h"

UENUM(BlueprintType)
enum class ESBUILayer : uint8
{
	GameHUD,
	Menu,
	PopupModal,
	Loading
};

USTRUCT(BlueprintType)
struct FWidgetStackWrapper
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<USBUserWidget>> Stack;
};

UCLASS(BlueprintType)
class SANDBOXUI_API USBUIManager : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	USBUIManager();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|UI")
	USBUserWidget* PushWidget(TSubclassOf<USBUserWidget> WidgetClass, ESBUILayer Layer);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|UI")
	void PopWidget(ESBUILayer Layer);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|UI")
	void ClearLayer(ESBUILayer Layer);

private:
	UPROPERTY()
	TMap<ESBUILayer, FWidgetStackWrapper> LayerStacks;
};
