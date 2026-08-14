#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Interfaces/SBSaveInterface.h"
#include "Utilities/SBLogCategories.h"

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
	if (!Object) return;

	TArray<uint8> BinaryData;
	FMemoryWriter Writer(BinaryData);
	FObjectAndNameAsStringProxyArchive Archive(Writer, true);
	Archive.ArIsSaveGame = true;

	Object->Serialize(Archive);

	WriteBinaryData(Key, BinaryData);
	UE_LOG(LogSandboxCore, Log, TEXT("USBSavePayload::SerializeObject: Key=%s, DataSize=%d"), *Key, BinaryData.Num());
}

void USBSavePayload::DeserializeObject(const FString& Key, UObject* Object)
{
	if (!Object) return;

	TArray<uint8> BinaryData;
	if (ReadBinaryData(Key, BinaryData) && BinaryData.Num() > 0)
	{
		UE_LOG(LogSandboxCore, Log, TEXT("USBSavePayload::DeserializeObject: Key=%s, DataSize=%d"), *Key, BinaryData.Num());
		FMemoryReader Reader(BinaryData);
		FObjectAndNameAsStringProxyArchive Archive(Reader, true);
		Archive.ArIsSaveGame = true;

		Object->Serialize(Archive);
	}
	else
	{
		UE_LOG(LogSandboxCore, Warning, TEXT("USBSavePayload::DeserializeObject: Key=%s NOT FOUND or empty!"), *Key);
	}
}

bool USBSaveSubsystemConcrete::SaveGame(const FString& SlotName, int32 UserIndex)
{
	UE_LOG(LogSandboxCore, Log, TEXT("USBSaveSubsystemConcrete::SaveGame: SlotName=%s, UserIndex=%d"), *SlotName, UserIndex);
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

	SaveGameObject->SaveVersion = 1;

	USBSavePayload* Payload = NewObject<USBSavePayload>(this);

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
	return UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, UserIndex);
}

bool USBSaveSubsystemConcrete::LoadGame(const FString& SlotName, int32 UserIndex)
{
	UE_LOG(LogSandboxCore, Log, TEXT("USBSaveSubsystemConcrete::LoadGame: SlotName=%s, UserIndex=%d"), *SlotName, UserIndex);
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::LoadGame: World is null!"));
		return false;
	}

	USBSaveGame* SaveGameObject = Cast<USBSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	if (!SaveGameObject)
	{
		UE_LOG(LogSandboxCore, Error, TEXT("USBSaveSubsystemConcrete::LoadGame: Failed to load USBSaveGame object from slot %s!"), *SlotName);
		return false;
	}

	if (SaveGameObject->SaveVersion > 1)
	{
		UE_LOG(LogSandboxCore, Warning, TEXT("USBSaveSubsystemConcrete::LoadGame: Save slot has a newer version (%d) than supported (1)!"), SaveGameObject->SaveVersion);
	}

	USBSavePayload* Payload = NewObject<USBSavePayload>(this);
	Payload->ObjectDataMap = SaveGameObject->SerializedObjects;

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
