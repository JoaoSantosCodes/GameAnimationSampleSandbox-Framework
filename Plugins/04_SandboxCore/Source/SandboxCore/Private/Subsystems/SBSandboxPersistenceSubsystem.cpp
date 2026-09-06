#include "Subsystems/SBSandboxPersistenceSubsystem.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Components/SBPersistenceComponent.h"
#include "Interfaces/SBSaveInterface.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

USBSandboxPersistenceSubsystem::USBSandboxPersistenceSubsystem()
{
}

void USBSandboxPersistenceSubsystem::SaveWorldState(UObject* SavePayload)
{
	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (!Payload)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (!Actor)
		{
			continue;
		}

		USBPersistenceComponent* PersistComp = Actor->FindComponentByClass<USBPersistenceComponent>();
		if (PersistComp && PersistComp->PersistentId.IsValid())
		{
			FGuid Guid = PersistComp->PersistentId.Guid;
			
			if (FSBSerializedActorData* ExistingData = SerializedActors.Find(Guid))
			{
				if (ExistingData->bWasDestroyed)
				{
					continue;
				}
			}

			FSBSerializedActorData ActorData;
			ActorData.Guid = Guid;
			ActorData.ActorClass = Actor->GetClass();
			ActorData.Transform = Actor->GetTransform();
			ActorData.bWasDestroyed = false;

			if (Actor->Implements<USBSaveInterface>())
			{
				USBSavePayload* TempPayload = NewObject<USBSavePayload>(this);
				ISBSaveInterface::Execute_SaveComponentData(Actor, TempPayload);

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
					ISBSaveInterface::Execute_SaveComponentData(Comp, TempPayload);
				}

				FMemoryWriter Writer(ActorData.BytePayload);
				FObjectAndNameAsStringProxyArchive Archive(Writer, true);
				Archive.ArIsSaveGame = true;
				TempPayload->Serialize(Archive);
			}

			SerializedActors.Add(Guid, ActorData);
		}
	}

	TArray<uint8> WorldBinary;
	FMemoryWriter WorldWriter(WorldBinary);
	FObjectAndNameAsStringProxyArchive WorldArchive(WorldWriter, true);
	WorldArchive.ArIsSaveGame = true;
	this->Serialize(WorldArchive);

	Payload->WriteBinaryData(TEXT("SandboxWorldState"), WorldBinary);
	UE_LOG(LogTemp, Log, TEXT("Sandbox Persistence: Saved %d actors to slot."), SerializedActors.Num());
}

void USBSandboxPersistenceSubsystem::LoadWorldState(UObject* SavePayload)
{
	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (!Payload)
	{
		return;
	}

	TArray<uint8> WorldBinary;
	if (!Payload->ReadBinaryData(TEXT("SandboxWorldState"), WorldBinary) || WorldBinary.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sandbox Persistence: No saved world state found."));
		return;
	}

	FMemoryReader WorldReader(WorldBinary);
	FObjectAndNameAsStringProxyArchive WorldArchive(WorldReader, true);
	WorldArchive.ArIsSaveGame = true;
	this->Serialize(WorldArchive);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TMap<FGuid, AActor*> LevelActorsMap;
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (Actor)
		{
			USBPersistenceComponent* PersistComp = Actor->FindComponentByClass<USBPersistenceComponent>();
			if (PersistComp && PersistComp->PersistentId.IsValid())
			{
				LevelActorsMap.Add(PersistComp->PersistentId.Guid, Actor);
			}
		}
	}

	for (auto& Elem : SerializedActors)
	{
		FGuid Guid = Elem.Key;
		const FSBSerializedActorData& ActorData = Elem.Value;

		if (ActorData.bWasDestroyed)
		{
			if (AActor** FoundActorPtr = LevelActorsMap.Find(Guid))
			{
				AActor* FoundActor = *FoundActorPtr;
				if (FoundActor)
				{
					FoundActor->Destroy();
				}
			}
			continue;
		}

		AActor* TargetActor = nullptr;
		if (AActor** FoundActorPtr = LevelActorsMap.Find(Guid))
		{
			TargetActor = *FoundActorPtr;
		}
		else if (ActorData.ActorClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			TargetActor = World->SpawnActor<AActor>(ActorData.ActorClass, ActorData.Transform, SpawnParams);
			
			if (TargetActor)
			{
				USBPersistenceComponent* NewPersistComp = TargetActor->FindComponentByClass<USBPersistenceComponent>();
				if (!NewPersistComp)
				{
					NewPersistComp = NewObject<USBPersistenceComponent>(TargetActor);
					NewPersistComp->RegisterComponent();
				}
				NewPersistComp->PersistentId.Guid = Guid;
			}
		}

		if (TargetActor)
		{
			TargetActor->SetActorTransform(ActorData.Transform);

			if (ActorData.BytePayload.Num() > 0 && TargetActor->Implements<USBSaveInterface>())
			{
				USBSavePayload* TempPayload = NewObject<USBSavePayload>(this);
				
				FMemoryReader Reader(ActorData.BytePayload);
				FObjectAndNameAsStringProxyArchive Archive(Reader, true);
				Archive.ArIsSaveGame = true;
				TempPayload->Serialize(Archive);

				ISBSaveInterface::Execute_LoadComponentData(TargetActor, TempPayload);

				TInlineComponentArray<UActorComponent*> SaveComponents(TargetActor);
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
					ISBSaveInterface::Execute_LoadComponentData(Comp, TempPayload);
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Sandbox Persistence: Loaded and synchronized %d actors."), SerializedActors.Num());
}

void USBSandboxPersistenceSubsystem::RecordActorDestruction(FGuid ActorGuid)
{
	if (!ActorGuid.IsValid())
	{
		return;
	}

	FSBSerializedActorData& ActorData = SerializedActors.FindOrAdd(ActorGuid);
	ActorData.Guid = ActorGuid;
	ActorData.bWasDestroyed = true;
	ActorData.BytePayload.Empty();
}
