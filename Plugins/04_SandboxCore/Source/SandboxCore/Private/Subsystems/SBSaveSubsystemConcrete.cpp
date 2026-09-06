#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Interfaces/SBSaveInterface.h"
#include "Utilities/SBLogCategories.h"
#include "Misc/SecureHash.h"
#include "Settings/SBDeveloperSettings.h"
#include "Subsystems/SBSandboxBackgroundSimSubsystem.h"
#include "Subsystems/SBWeatherSubsystem.h"

void USBSavePayload::WriteBinaryData(const FString& Key, const TArray<uint8>& Data)
{
	FSBSaveObjectData Wrapper;
	Wrapper.ByteData = Data;
	ObjectDataMap.Add(Key, Wrapper);
}

bool USBSavePayload::ReadBinaryData(const FString& Key, TArray<uint8>& OutData) const
{
	if (const FSBSaveObjectData* Data = ObjectDataMap.Find(Key))
	{
		OutData = Data->ByteData;
		return true;
	}
	return false;
}

void USBSavePayload::SerializeObject(const FString& Key, UObject* Object)
{
	if (!Object)
	{
		return;
	}

	TArray<uint8> ObjectBytes;
	FMemoryWriter MemoryWriter(ObjectBytes, true);
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, false);
	Archive.ArIsSaveGame = true;

	Object->Serialize(Archive);

	WriteBinaryData(Key, ObjectBytes);
}

void USBSavePayload::DeserializeObject(const FString& Key, UObject* Object)
{
	if (!Object)
	{
		return;
	}

	TArray<uint8> ObjectBytes;
	if (ReadBinaryData(Key, ObjectBytes))
	{
		FMemoryReader MemoryReader(ObjectBytes, true);
		FObjectAndNameAsStringProxyArchive Archive(MemoryReader, false);
		Archive.ArIsSaveGame = true;

		Object->Serialize(Archive);
	}
}

void USBSaveSubsystemConcrete::RegisterMigrationStep(int32 FromVersion, int32 ToVersion, FSBMigrationFunction MigrationFunc, const FString& Description)
{
	FSBSaveMigrationStep Step;
	Step.FromVersion = FromVersion;
	Step.ToVersion = ToVersion;
	Step.MigrationFunction = MigrationFunc;
	Step.Description = Description;
	RegisteredMigrationSteps.Add(Step);
}

bool USBSaveSubsystemConcrete::MigratePayload(USBSavePayload* Payload, int32& InOutVersion, int32 TargetVersion)
{
	if (!Payload)
	{
		return false;
	}

	while (InOutVersion < TargetVersion)
	{
		const FSBSaveMigrationStep* Step = RegisteredMigrationSteps.FindByPredicate([InOutVersion](const FSBSaveMigrationStep& S)
		{
			return S.FromVersion == InOutVersion;
		});

		if (!Step)
		{
			UE_LOG(LogSandboxCore, Warning, TEXT("USBSaveSubsystemConcrete::MigratePayload: No explicit migration step for %d -> %d. Assuming backward compatibility."), InOutVersion, InOutVersion + 1);
			InOutVersion++;
			continue;
		}

		if (Step->MigrationFunction)
		{
			bool bSuccess = Step->MigrationFunction(Payload);
			if (!bSuccess)
			{
				UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::MigratePayload: Migration step failed from %d to %d (%s)!"), Step->FromVersion, Step->ToVersion, *Step->Description);
				return false;
			}
		}

		InOutVersion = Step->ToVersion;
	}

	return true;
}

bool USBSaveSubsystemConcrete::SaveGame(const FString& SlotName, int32 UserIndex)
{
	UE_LOG(LogSandboxCore, Log, TEXT("USBSaveSubsystemConcrete::SaveGame (Secure): SlotName=%s, UserIndex=%d"), *SlotName, UserIndex);
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::SaveGame: World is null!"));
		return false;
	}

	USBSaveGame* SaveGameObject = Cast<USBSaveGame>(UGameplayStatics::CreateSaveGameObject(USBSaveGame::StaticClass()));
	if (!SaveGameObject)
	{
		UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::SaveGame: Failed to create USBSaveGame object!"));
		return false;
	}

	SaveGameObject->SaveVersion = CURRENT_SAVE_VERSION;

	USBSavePayload* Payload = NewObject<USBSavePayload>(this);

	if (USBSandboxBackgroundSimSubsystem* SimSubsystem = World->GetSubsystem<USBSandboxBackgroundSimSubsystem>())
	{
		ISBSaveInterface::Execute_SaveComponentData(SimSubsystem, Payload);
	}

	if (USBWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<USBWeatherSubsystem>())
	{
		ISBSaveInterface::Execute_SaveComponentData(WeatherSubsystem, Payload);
	}

	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (Actor)
		{
			if (Actor->Implements<USBSaveInterface>())
			{
				UE_LOG(LogSandboxCore, Log, TEXT("USBSaveSubsystemConcrete::SaveGame: Saving actor %s"), *Actor->GetPathName());
				ISBSaveInterface::Execute_SaveComponentData(Actor, Payload);
			}

			TInlineComponentArray<UActorComponent*> SaveComponents(Actor);
			TArray<UActorComponent*> InterfaceComponents;
			for (UActorComponent* Comp : SaveComponents)
			{
				if (Comp && Comp->Implements<USBSaveInterface>())
				{
					InterfaceComponents.Add(Comp);
				}
			}

			InterfaceComponents.Sort([](const UActorComponent& A, const UActorComponent& B)
			{
				int32 PriorityA = ISBSaveInterface::Execute_GetSavePriority(const_cast<UActorComponent*>(&A));
				int32 PriorityB = ISBSaveInterface::Execute_GetSavePriority(const_cast<UActorComponent*>(&B));
				return PriorityA > PriorityB;
			});

			for (UActorComponent* Comp : InterfaceComponents)
			{
				UE_LOG(LogSandboxCore, Log, TEXT("USBSaveSubsystemConcrete::SaveGame: Saving component %s"), *Comp->GetPathName());
				ISBSaveInterface::Execute_SaveComponentData(Comp, Payload);
			}
		}
	}

	SaveGameObject->SerializedObjects = Payload->ObjectDataMap;

	// 1. Serializa o SaveGameObject em memória
	TArray<uint8> RawBytes;
	if (!UGameplayStatics::SaveGameToMemory(SaveGameObject, RawBytes))
	{
		UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::SaveGame: Failed to serialize SaveGameObject to memory!"));
		return false;
	}

	// 2. Criptografa o Payload na memória
	const USBDeveloperSettings* Settings = GetDefault<USBDeveloperSettings>();
	FString SecretKey = Settings ? Settings->SaveEncryptionKey : TEXT("SandboxAntiSaveScummingKey2026SecureSalt");
	EncryptDecryptData(RawBytes, SecretKey);

	// 3. Calcula a assinatura digital (HMAC-MD5)
	FString Signature = CalculateSignature(RawBytes, SecretKey);

	// 4. Cria e persiste o USBSecureSaveGame
	USBSecureSaveGame* SecureSave = Cast<USBSecureSaveGame>(UGameplayStatics::CreateSaveGameObject(USBSecureSaveGame::StaticClass()));
	if (!SecureSave)
	{
		UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::SaveGame: Failed to create USBSecureSaveGame!"));
		return false;
	}

	SecureSave->EncryptedPayload = RawBytes;
	SecureSave->Signature = Signature;

	return UGameplayStatics::SaveGameToSlot(SecureSave, SlotName, UserIndex);
}

bool USBSaveSubsystemConcrete::LoadGame(const FString& SlotName, int32 UserIndex)
{
	UE_LOG(LogSandboxCore, Log, TEXT("USBSaveSubsystemConcrete::LoadGame (Secure): SlotName=%s, UserIndex=%d"), *SlotName, UserIndex);
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::LoadGame: World is null!"));
		return false;
	}

	// 1. Carrega o container seguro
	USBSecureSaveGame* SecureSave = Cast<USBSecureSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	if (!SecureSave)
	{
		UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::LoadGame: Failed to load USBSecureSaveGame object from slot %s!"), *SlotName);
		return false;
	}

	// 2. Verifica a integridade da assinatura (Anti-Save Scumming)
	const USBDeveloperSettings* Settings = GetDefault<USBDeveloperSettings>();
	FString SecretKey = Settings ? Settings->SaveEncryptionKey : TEXT("SandboxAntiSaveScummingKey2026SecureSalt");
	FString CalculatedSignature = CalculateSignature(SecureSave->EncryptedPayload, SecretKey);

	if (CalculatedSignature != SecureSave->Signature)
	{
		UE_LOG(LogSandboxCore, Log, TEXT("=================================================================================="));
		UE_LOG(LogSandboxCore, Log, TEXT("SECURITY WARNING: SAVE GAME FILE '%s' IS CORRUPTED OR HAS BEEN ILLEGALLY MODIFIED!"), *SlotName);
		UE_LOG(LogSandboxCore, Log, TEXT("LOAD ABORTED (Anti-Save Scumming Protection Active)"));
		UE_LOG(LogSandboxCore, Log, TEXT("=================================================================================="));
		return false;
	}

	// 3. Decifra o payload
	TArray<uint8> DecryptedBytes = SecureSave->EncryptedPayload;
	EncryptDecryptData(DecryptedBytes, SecretKey);

	// 4. Deserializa o USBSaveGame original da memória
	USBSaveGame* SaveGameObject = Cast<USBSaveGame>(UGameplayStatics::LoadGameFromMemory(DecryptedBytes));
	if (!SaveGameObject)
	{
		UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::LoadGame: Failed to deserialize USBSaveGame from decrypted bytes!"));
		return false;
	}

	USBSavePayload* Payload = NewObject<USBSavePayload>(this);
	Payload->ObjectDataMap = SaveGameObject->SerializedObjects;

	// 5. Migra o payload se a versão for anterior à versão atual suportada
	if (SaveGameObject->SaveVersion < CURRENT_SAVE_VERSION)
	{
		UE_LOG(LogSandboxCore, Log, TEXT("USBSaveSubsystemConcrete::LoadGame: Save slot '%s' has version %d, migrating to version %d..."), *SlotName, SaveGameObject->SaveVersion, CURRENT_SAVE_VERSION);

		int32 OldVersion = SaveGameObject->SaveVersion;
		if (!MigratePayload(Payload, SaveGameObject->SaveVersion, CURRENT_SAVE_VERSION))
		{
			UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::LoadGame: Migration failed for slot '%s'!"), *SlotName);
			return false;
		}

		UE_LOG(LogSandboxCore, Log, TEXT("USBSaveSubsystemConcrete::LoadGame: Successfully migrated slot '%s' from version %d to %d."), *SlotName, OldVersion, SaveGameObject->SaveVersion);
		SaveGameObject->SerializedObjects = Payload->ObjectDataMap;
	}
	else if (SaveGameObject->SaveVersion > CURRENT_SAVE_VERSION)
	{
		UE_LOG(LogSandboxCore, Warning, TEXT("USBSaveSubsystemConcrete::LoadGame: Save slot '%s' has a newer version (%d) than supported (%d)!"), *SlotName, SaveGameObject->SaveVersion, CURRENT_SAVE_VERSION);
	}

	if (USBSandboxBackgroundSimSubsystem* SimSubsystem = World->GetSubsystem<USBSandboxBackgroundSimSubsystem>())
	{
		ISBSaveInterface::Execute_LoadComponentData(SimSubsystem, Payload);
	}

	if (USBWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<USBWeatherSubsystem>())
	{
		ISBSaveInterface::Execute_LoadComponentData(WeatherSubsystem, Payload);
	}

	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (Actor)
		{
			if (Actor->Implements<USBSaveInterface>())
			{
				UE_LOG(LogSandboxCore, Log, TEXT("USBSaveSubsystemConcrete::LoadGame: Loading actor %s"), *Actor->GetPathName());
				ISBSaveInterface::Execute_LoadComponentData(Actor, Payload);
			}

			TInlineComponentArray<UActorComponent*> SaveComponents(Actor);
			TArray<UActorComponent*> InterfaceComponents;
			for (UActorComponent* Comp : SaveComponents)
			{
				if (Comp && Comp->Implements<USBSaveInterface>())
				{
					InterfaceComponents.Add(Comp);
				}
			}

			InterfaceComponents.Sort([](const UActorComponent& A, const UActorComponent& B)
			{
				int32 PriorityA = ISBSaveInterface::Execute_GetSavePriority(const_cast<UActorComponent*>(&A));
				int32 PriorityB = ISBSaveInterface::Execute_GetSavePriority(const_cast<UActorComponent*>(&B));
				return PriorityA > PriorityB;
			});

			for (UActorComponent* Comp : InterfaceComponents)
			{
				UE_LOG(LogSandboxCore, Log, TEXT("USBSaveSubsystemConcrete::LoadGame: Loading component %s"), *Comp->GetPathName());
				ISBSaveInterface::Execute_LoadComponentData(Comp, Payload);
			}
		}
	}

	return true;
}

void USBSaveSubsystemConcrete::EncryptDecryptData(TArray<uint8>& Data, const FString& Key)
{
	if (Data.Num() == 0 || Key.IsEmpty())
	{
		return;
	}

	FTCHARToUTF8 Converter(*Key);
	const uint8* KeyBytes = reinterpret_cast<const uint8*>(Converter.Get());
	int32 KeyLength = Converter.Length();

	if (KeyLength == 0)
	{
		return;
	}

	for (int32 i = 0; i < Data.Num(); ++i)
	{
		Data[i] ^= KeyBytes[i % KeyLength];
	}
}

FString USBSaveSubsystemConcrete::CalculateSignature(const TArray<uint8>& Data, const FString& Key)
{
	if (Data.Num() == 0 || Key.IsEmpty())
	{
		return TEXT("");
	}

	TArray<uint8> CombinedData;
	CombinedData.Append(Data);

	FTCHARToUTF8 KeyConverter(*Key);
	CombinedData.Append(reinterpret_cast<const uint8*>(KeyConverter.Get()), KeyConverter.Length());

	return FMD5::HashBytes(CombinedData.GetData(), CombinedData.Num());
}
