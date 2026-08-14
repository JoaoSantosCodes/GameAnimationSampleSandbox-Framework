#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Types/SBCommonTypes.h"
#include "SBEventSubsystem.generated.h"

DECLARE_DELEGATE_TwoParams(FSBNativeEventDelegate, FGameplayTag, UObject*);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FSBBlueprintEventDelegate, FGameplayTag, EventTag, UObject*, Payload);

USTRUCT(BlueprintType)
struct FSBBlueprintListener
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Events")
	ESBEventPriority Priority = ESBEventPriority::Medium;

	UPROPERTY(BlueprintReadWrite, Category = "Events")
	FSBBlueprintEventDelegate Delegate;
};

USTRUCT(BlueprintType)
struct FSBBlueprintListenerArray
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Events")
	TArray<FSBBlueprintListener> Listeners;
};

struct FSBNativeListener
{
	ESBEventPriority Priority = ESBEventPriority::Medium;
	FSBNativeEventDelegate Delegate;
	FDelegateHandle Handle;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USBEventSubsystem();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Events")
	void PublishEvent(FGameplayTag EventTag, UObject* Payload);

	FDelegateHandle SubscribeToEventNative(FGameplayTag EventTag, ESBEventPriority Priority, FSBNativeEventDelegate Delegate);

	void UnsubscribeFromEventNative(FGameplayTag EventTag, FDelegateHandle Handle);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Events")
	void SubscribeToEvent(FGameplayTag EventTag, ESBEventPriority Priority, FSBBlueprintEventDelegate Delegate);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Events")
	void UnsubscribeFromEvent(FGameplayTag EventTag, FSBBlueprintEventDelegate Delegate);

private:
	TMap<FGameplayTag, TArray<FSBNativeListener>> NativeListeners;

	UPROPERTY()
	TMap<FGameplayTag, FSBBlueprintListenerArray> BlueprintListeners;
};
