#include "Components/SBSpatialIndexedComponent.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "Subsystems/SBSpatialPartitionSubsystem.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBSpatialIndexedComponent::USBSpatialIndexedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	EntityID = 0;
	EntityType = 0;
	BoundingRadius = 50.0f;
	LastRegisteredLocation = FVector::ZeroVector;
	CurrentCell = FSBSpatialCellCoord();
	bIsRegistered = false;
}

void USBSpatialIndexedComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass());
		LastRegisteredLocation = Owner->GetActorLocation();
	}

	if (EntityID != 0)
	{
		SyncLocationToSpatialGrid();
	}
}

void USBSpatialIndexedComponent::OnShutdown_Implementation()
{
	if (bIsRegistered && GetWorld())
	{
		if (USBSpatialPartitionSubsystem* SpatialSubsystem = GetWorld()->GetSubsystem<USBSpatialPartitionSubsystem>())
		{
			SpatialSubsystem->UnregisterEntity(EntityID, LastRegisteredLocation);
		}
	}
	bIsRegistered = false;
}

void USBSpatialIndexedComponent::SetupSpatialEntity(int32 InEntityID, int32 InEntityType, float InRadius)
{
	EntityID = InEntityID;
	EntityType = InEntityType;
	BoundingRadius = InRadius;

	SyncLocationToSpatialGrid();
}

void USBSpatialIndexedComponent::SyncLocationToSpatialGrid()
{
	if (!GetWorld() || EntityID == 0)
	{
		return;
	}

	USBSpatialPartitionSubsystem* SpatialSubsystem = GetWorld()->GetSubsystem<USBSpatialPartitionSubsystem>();
	if (!SpatialSubsystem)
	{
		return;
	}

	const FVector CurrentLocation = GetOwner() ? GetOwner()->GetActorLocation() : LastRegisteredLocation;
	const FSBSpatialCellCoord NewCell = SpatialSubsystem->WorldToCell(CurrentLocation);

	if (!bIsRegistered)
	{
		FSBSpatialEntityElement Element;
		Element.EntityID = EntityID;
		Element.EntityType = EntityType;
		Element.Location = CurrentLocation;
		Element.BoundingRadius = BoundingRadius;

		SpatialSubsystem->RegisterEntity(Element);
		CurrentCell = NewCell;
		LastRegisteredLocation = CurrentLocation;
		bIsRegistered = true;
	}
	else
	{
		const FSBSpatialCellCoord OldCell = CurrentCell;
		SpatialSubsystem->UpdateEntityLocation(EntityID, LastRegisteredLocation, CurrentLocation);
		CurrentCell = NewCell;
		LastRegisteredLocation = CurrentLocation;

		if (OldCell != NewCell)
		{
			OnSpatialCellChanged.Broadcast(OldCell.X, OldCell.Y, OldCell.Z, NewCell.X, NewCell.Y, NewCell.Z);
		}
	}

	SyncTags();
}

void USBSpatialIndexedComponent::SyncTags()
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

	if (bIsRegistered)
	{
		ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Spatial_Indexed);
		ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Spatial_CellActive);
	}
	else
	{
		ISBStateComponentInterface::Execute_RemoveTag(CachedStateComp.Get(), Tags.State_Spatial_Indexed);
		ISBStateComponentInterface::Execute_RemoveTag(CachedStateComp.Get(), Tags.State_Spatial_CellActive);
	}
}
