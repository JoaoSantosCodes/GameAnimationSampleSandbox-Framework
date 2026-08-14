#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBUserWidget.h"
#include "SBStatusHUDWidget.generated.h"

class UProgressBar;

UCLASS(Abstract, Blueprintable)
class SANDBOXUI_API USBStatusHUDWidget : public USBUserWidget
{
	GENERATED_BODY()

public:
	USBStatusHUDWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnAttributeChanged(FGameplayTag EventTag, UObject* Payload);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Sandbox|UI")
	TObjectPtr<UProgressBar> PB_Health;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Sandbox|UI")
	TObjectPtr<UProgressBar> PB_Mana;
};
