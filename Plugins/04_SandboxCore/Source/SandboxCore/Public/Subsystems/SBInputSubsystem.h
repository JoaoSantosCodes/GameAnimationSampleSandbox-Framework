#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "SBInputSubsystem.generated.h"

class UInputMappingContext;

UCLASS()
class SANDBOXCORE_API USBInputSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Input")
	void AddMappingContext(const UInputMappingContext* MappingContext, int32 Priority);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Input")
	void RemoveMappingContext(const UInputMappingContext* MappingContext);
};
