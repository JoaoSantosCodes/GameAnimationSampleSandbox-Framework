#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SBDebugInterface.generated.h"

USTRUCT(BlueprintType)
struct FSBDebugLine
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Sandbox|Debug")
	FString Label;

	UPROPERTY(BlueprintReadOnly, Category = "Sandbox|Debug")
	FString Value;

	UPROPERTY(BlueprintReadOnly, Category = "Sandbox|Debug")
	bool bIsHeader = false;
};

UINTERFACE(MinimalAPI, BlueprintType)
class USBDebugInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBDebugInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Sandbox|Debug")
	void GetDebugDescription(TArray<FSBDebugLine>& OutDebugLines) const;
};
