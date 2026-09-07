// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBAsyncSerializableComponent.h"
#include "Subsystems/SBAsyncSerializationSubsystem.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "SBGameplayTags.h"
#include "Misc/DateTime.h"
#include "GameFramework/Actor.h"

USBAsyncSerializableComponent::USBAsyncSerializableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBAsyncSerializableComponent::OnInitialize_Implementation()
{
	if (!UniquePersistentGuid.IsValid())
	{
		UniquePersistentGuid = FGuid::NewGuid();
	}
	SyncTags();
}

void USBAsyncSerializableComponent::OnReady_Implementation()
{
	SyncTags();
}

void USBAsyncSerializableComponent::OnShutdown_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Save_Serialized);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Save_AsyncSaving);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Save_AsyncLoading);
		}
	}
}

FSBAsyncSaveRecord USBAsyncSerializableComponent::CaptureSaveRecord(const TArray<uint8>& CustomPayload)
{
	if (!UniquePersistentGuid.IsValid())
	{
		UniquePersistentGuid = FGuid::NewGuid();
	}

	FSBAsyncSaveRecord Record;
	Record.EntityGuid = UniquePersistentGuid;
	Record.RecordTag = SaveCategory;
	Record.BinaryData = CustomPayload;
	Record.ChecksumHash = USBAsyncSerializationSubsystem::ComputePayloadHash(CustomPayload);
	Record.TimestampTicks = FDateTime::UtcNow().GetTicks();

	LastBinaryData = CustomPayload;
	SyncTags();

	return Record;
}

bool USBAsyncSerializableComponent::ApplyLoadRecord(const FSBAsyncSaveRecord& InRecord)
{
	if (UniquePersistentGuid.IsValid() && InRecord.EntityGuid != UniquePersistentGuid)
	{
		return false;
	}

	FString CalculatedHash = USBAsyncSerializationSubsystem::ComputePayloadHash(InRecord.BinaryData);
	if (!InRecord.ChecksumHash.IsEmpty() && CalculatedHash != InRecord.ChecksumHash)
	{
		return false;
	}

	UniquePersistentGuid = InRecord.EntityGuid;
	SaveCategory = InRecord.RecordTag;
	LastBinaryData = InRecord.BinaryData;
	SyncTags();

	return true;
}

void USBAsyncSerializableComponent::SyncTags()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	if (!StateComp)
	{
		return;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (LastBinaryData.Num() > 0)
	{
		ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Save_Serialized);
	}
	else
	{
		ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Save_Serialized);
	}
}
