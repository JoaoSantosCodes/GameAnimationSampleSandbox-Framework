#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBUserWidget.h"
#include "SBAbilityBarWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SANDBOXUI_API USBAbilityBarWidget : public USBUserWidget
{
	GENERATED_BODY()

public:
	USBAbilityBarWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void OnCooldownStarted(FGameplayTag EventTag, UObject* Payload);

	UFUNCTION()
	void OnCooldownEnded(FGameplayTag EventTag, UObject* Payload);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Sandbox|UI")
	TObjectPtr<UImage> IMG_CooldownMask;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true), Category = "Sandbox|UI")
	TObjectPtr<UTextBlock> TXT_CooldownTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|UI")
	FGameplayTag WatchedAbilityTag;

	float CooldownDuration = 0.0f;
	float CooldownRemaining = 0.0f;
	bool bIsCooldownActive = false;
};
