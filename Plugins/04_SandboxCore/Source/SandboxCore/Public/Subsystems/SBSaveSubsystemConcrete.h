#pragma once

#include "CoreMinimal.h"
#include "Subsystems/SBSaveSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "SBSaveSubsystemConcrete.generated.h"

USTRUCT(BlueprintType)
struct FSBSaveObjectData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<uint8> ByteData;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	int32 SaveVersion = 1;

	UPROPERTY(SaveGame)
	TMap<FString, FSBSaveObjectData> SerializedObjects;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBSavePayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FString, FSBSaveObjectData> ObjectDataMap;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Save")
	void WriteBinaryData(const FString& Key, const TArray<uint8>& Data);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Save")
	bool ReadBinaryData(const FString& Key, TArray<uint8>& OutData) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Save")
	void SerializeObject(const FString& Key, UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Save")
	void DeserializeObject(const FString& Key, UObject* Object);
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBSaveSubsystemConcrete : public USBSaveSubsystem
{
	GENERATED_BODY()

public:
	virtual bool SaveGame(const FString& SlotName, int32 UserIndex) override;
	virtual bool LoadGame(const FString& SlotName, int32 UserIndex) override;
};
