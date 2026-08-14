#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBUserWidget.h"
#include "SBInteractionPromptWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SANDBOXUI_API USBInteractionPromptWidget : public USBUserWidget
{
	GENERATED_BODY()

public:
	USBInteractionPromptWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnInteractionAvailable(FGameplayTag EventTag, UObject* Payload);

	UFUNCTION()
	void OnInteractionCleared(FGameplayTag EventTag, UObject* Payload);

	UFUNCTION()
	void OnInteractionProgress(FGameplayTag EventTag, UObject* Payload);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Sandbox|UI")
	TObjectPtr<UTextBlock> TXT_Prompt;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Sandbox|UI")
	TObjectPtr<UProgressBar> PB_HoldProgress;
};
