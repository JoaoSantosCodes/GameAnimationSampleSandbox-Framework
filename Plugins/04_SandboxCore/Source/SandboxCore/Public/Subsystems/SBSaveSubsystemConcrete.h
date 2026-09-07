// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/SBSaveSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "Templates/Function.h"
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

using FSBMigrationFunction = TFunction<bool(USBSavePayload* Payload)>;

USTRUCT(BlueprintType)
struct SANDBOXCORE_API FSBSaveMigrationStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveMigration")
	int32 FromVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveMigration")
	int32 ToVersion = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveMigration")
	FString Description;

	FSBMigrationFunction MigrationFunction;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBSecureSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	TArray<uint8> EncryptedPayload;

	UPROPERTY(SaveGame)
	FString Signature;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBSaveSubsystemConcrete : public USBSaveSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 CURRENT_SAVE_VERSION = 2;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Save")
	int32 GetCurrentSaveVersion() const { return CURRENT_SAVE_VERSION; }

	// Registra um passo de migração customizado
	void RegisterMigrationStep(int32 FromVersion, int32 ToVersion, FSBMigrationFunction MigrationFunc, const FString& Description = TEXT(""));

	// Executa a cadeia de migrações aplicáveis sequencialmente no payload
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Save")
	bool MigratePayload(USBSavePayload* Payload, UPARAM(ref) int32& InOutVersion, int32 TargetVersion);

	virtual bool SaveGame(const FString& SlotName, int32 UserIndex) override;
	virtual bool LoadGame(const FString& SlotName, int32 UserIndex) override;

private:
	void EncryptDecryptData(TArray<uint8>& Data, const FString& Key);
	FString CalculateSignature(const TArray<uint8>& Data, const FString& Key);

	TArray<FSBSaveMigrationStep> RegisteredMigrationSteps;
};
