#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SBSaveSubsystem.generated.h"

UCLASS(Abstract, BlueprintType)
class SANDBOXINTERFACES_API USBSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Save")
	virtual bool SaveGame(const FString& SlotName, int32 UserIndex) { return false; }

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Save")
	virtual bool LoadGame(const FString& SlotName, int32 UserIndex) { return false; }
};
