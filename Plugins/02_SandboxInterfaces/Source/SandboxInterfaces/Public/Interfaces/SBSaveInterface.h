#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SBSaveInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBSaveInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBSaveInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Save")
	bool SaveComponentData(UObject* SavePayload);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Save")
	bool LoadComponentData(UObject* SavePayload);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Save")
	int32 GetSavePriority() const;
	virtual int32 GetSavePriority_Implementation() const { return 0; }
};
