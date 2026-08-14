#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBUserWidget.h"
#include "SBInventoryGridWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class SANDBOXUI_API USBInventoryGridWidget : public USBUserWidget
{
	GENERATED_BODY()

public:
	USBInventoryGridWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnSlotUpdated(FGameplayTag EventTag, UObject* Payload);

	UFUNCTION(BlueprintImplementableEvent, Category = "Sandbox|UI")
	void BP_OnSlotUpdated(UObject* ItemInstance);
};
