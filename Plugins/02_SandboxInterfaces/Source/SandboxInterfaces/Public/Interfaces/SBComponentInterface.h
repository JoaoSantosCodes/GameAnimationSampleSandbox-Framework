#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SBComponentInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBComponentInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBComponentInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Component Lifecycle")
	void OnComponentCreated();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Component Lifecycle")
	void OnPreInitialize();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Component Lifecycle")
	void OnInitialize();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Component Lifecycle")
	void OnPostInitialize();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Component Lifecycle")
	void OnReady();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Component Lifecycle")
	void OnShutdown();
};
