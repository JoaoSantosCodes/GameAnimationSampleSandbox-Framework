#pragma once

#include "CoreMinimal.h"
#include "Widgets/SBUserWidget.h"
#include "SBUITestMockWidget.generated.h"

UCLASS(Blueprintable, BlueprintType)
class SANDBOXUI_API USBUITestMockWidget : public USBUserWidget
{
	GENERATED_BODY()

public:
	USBUITestMockWidget(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "Test")
	int32 CallCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Test")
	TObjectPtr<UObject> LastPayload = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Test")
	void HandleTestEvent(FGameplayTag EventTag, UObject* Payload)
	{
		CallCount++;
		LastPayload = Payload;
	}
};
