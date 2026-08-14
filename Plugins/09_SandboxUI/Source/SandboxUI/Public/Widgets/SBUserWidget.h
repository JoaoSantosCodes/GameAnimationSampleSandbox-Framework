#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "SBUserWidget.generated.h"

USTRUCT(BlueprintType)
struct FSBWidgetEventSubscription
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag EventTag;

	UPROPERTY()
	FSBBlueprintEventDelegate Delegate;
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class SANDBOXUI_API USBUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USBUserWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox|UI")
	void OnPushed();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox|UI")
	void OnPopped();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|UI")
	void SubscribeToEvent(FGameplayTag EventTag, FSBBlueprintEventDelegate Delegate);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|UI")
	void UnsubscribeAllEvents();

	APawn* GetOwningPlayerPawn() const;

#if WITH_EDITOR
	bool bMockOwningPawn = false;
	TObjectPtr<APawn> MockOwningPawn = nullptr;
#endif

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(Transient)
	TArray<FSBWidgetEventSubscription> Subscriptions;
};
