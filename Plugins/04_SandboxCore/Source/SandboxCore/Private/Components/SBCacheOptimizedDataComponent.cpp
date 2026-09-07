// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBCacheOptimizedDataComponent.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "Subsystems/SBCacheOptimizedBufferSubsystem.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBCacheOptimizedDataComponent::USBCacheOptimizedDataComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	BufferIndex = INDEX_NONE;
	EntityID = 0;
	TypeID = 0;
	CustomData = 0.0f;
}

void USBCacheOptimizedDataComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	}

	if (UWorld* World = GetWorld())
	{
		if (USBCacheOptimizedBufferSubsystem* BufferSubsystem = World->GetSubsystem<USBCacheOptimizedBufferSubsystem>())
		{
			FSBCompactEntityRecord Record;
			Record.EntityID = EntityID;
			Record.TypeID = TypeID;
			Record.CustomData = CustomData;

			if (AActor* Owner = GetOwner())
			{
				const FVector Loc = Owner->GetActorLocation();
				Record.PositionX = Loc.X;
				Record.PositionY = Loc.Y;
				Record.PositionZ = Loc.Z;
			}

			BufferIndex = BufferSubsystem->InsertRecord(Record);
		}
	}

	SyncTags();
}

void USBCacheOptimizedDataComponent::OnShutdown_Implementation()
{
	if (BufferIndex != INDEX_NONE)
	{
		if (UWorld* World = GetWorld())
		{
			if (USBCacheOptimizedBufferSubsystem* BufferSubsystem = World->GetSubsystem<USBCacheOptimizedBufferSubsystem>())
			{
				int32 SwappedEntityID = -1;
				BufferSubsystem->RemoveRecord(BufferIndex, SwappedEntityID);
			}
		}
		BufferIndex = INDEX_NONE;
	}
}

void USBCacheOptimizedDataComponent::SetupRecord(int32 InEntityID, int32 InTypeID)
{
	EntityID = InEntityID;
	TypeID = InTypeID;

	if (BufferIndex != INDEX_NONE)
	{
		SyncTransformToBuffer();
	}
}

void USBCacheOptimizedDataComponent::SyncTransformToBuffer()
{
	if (BufferIndex == INDEX_NONE)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (USBCacheOptimizedBufferSubsystem* BufferSubsystem = World->GetSubsystem<USBCacheOptimizedBufferSubsystem>())
		{
			FSBCompactEntityRecord Record;
			Record.EntityID = EntityID;
			Record.TypeID = TypeID;
			Record.CustomData = CustomData;

			if (AActor* Owner = GetOwner())
			{
				const FVector Loc = Owner->GetActorLocation();
				Record.PositionX = Loc.X;
				Record.PositionY = Loc.Y;
				Record.PositionZ = Loc.Z;
			}

			BufferSubsystem->UpdateRecord(BufferIndex, Record);
		}
	}
}

void USBCacheOptimizedDataComponent::SetCustomData(float InCustomData)
{
	CustomData = InCustomData;
	SyncTransformToBuffer();
}

FSBCompactEntityRecord USBCacheOptimizedDataComponent::GetCurrentRecord() const
{
	if (BufferIndex != INDEX_NONE)
	{
		if (UWorld* World = GetWorld())
		{
			if (USBCacheOptimizedBufferSubsystem* BufferSubsystem = World->GetSubsystem<USBCacheOptimizedBufferSubsystem>())
			{
				return BufferSubsystem->GetRecord(BufferIndex);
			}
		}
	}
	return FSBCompactEntityRecord();
}

void USBCacheOptimizedDataComponent::UpdateBufferIndexInternal(int32 NewIndex)
{
	BufferIndex = NewIndex;
	OnRecordIndexUpdated.Broadcast(NewIndex);
}

void USBCacheOptimizedDataComponent::SyncTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Memory_Optimized);
	ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Memory_Contiguous);
}
