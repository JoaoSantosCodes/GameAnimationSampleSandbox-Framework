#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SBEquippableInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBEquippableInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBEquippableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	UClass* GetWeaponBehaviorClass() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	UPrimaryDataAsset* GetWeaponDefinitionAsset() const;
};

UINTERFACE(MinimalAPI, BlueprintType)
class USBEquipEventPayloadInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBEquipEventPayloadInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	UObject* GetEquippableFragment() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	UObject* GetItemInstance() const;
};
