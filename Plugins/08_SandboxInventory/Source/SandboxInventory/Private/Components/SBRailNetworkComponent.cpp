#include "Components/SBRailNetworkComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBRailNetworkComponent::USBRailNetworkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBRailNetworkComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SimulateRailTick(0.0f);
}

void USBRailNetworkComponent::SetupTrainConsist(FName InTrainId, float InMaxSpeed)
{
	TrainConsist.TrainId = InTrainId;
	TrainConsist.MaxSpeed = InMaxSpeed;
	TrainConsist.CurrentSpeed = 0.0f;
	TrainConsist.CurrentTrackProgressAlpha = 0.0f;
	TrainConsist.CurrentTrackSegmentId = 0;
	TrainConsist.MovementState = ESBTrainMovementState::Stationary;
	SyncTrainState(ESBTrainMovementState::Stationary);
}

void USBRailNetworkComponent::AddWagon(const FSBRailWagonData& InWagon)
{
	TrainConsist.Wagons.Add(InWagon);
}

void USBRailNetworkComponent::AddStationToSchedule(FName InStationName)
{
	TrainConsist.DestinationStations.Add(InStationName);
}

void USBRailNetworkComponent::StartTravel()
{
	SyncTrainState(ESBTrainMovementState::Traveling);
}

void USBRailNetworkComponent::RegisterRailBlock(int32 InBlockId)
{
	FSBRailBlockData NewBlock;
	NewBlock.BlockId = InBlockId;
	NewBlock.bIsOccupied = false;
	NewBlock.OccupyingTrainId = NAME_None;
	NewBlock.SignalState = ESBRailSignalState::ClearGreen;
	RailBlocks.Add(InBlockId, NewBlock);
}

void USBRailNetworkComponent::SetBlockOccupied(int32 InBlockId, bool bOccupied, FName InOccupyingTrainId)
{
	if (FSBRailBlockData* Block = RailBlocks.Find(InBlockId))
	{
		Block->bIsOccupied = bOccupied;
		Block->OccupyingTrainId = InOccupyingTrainId;
		Block->SignalState = bOccupied ? ESBRailSignalState::StopRed : ESBRailSignalState::ClearGreen;
		OnRailSignalChanged.Broadcast(InBlockId, Block->SignalState);
	}
}

bool USBRailNetworkComponent::RequestBlockReservation(int32 InBlockId, FName InTrainId)
{
	if (FSBRailBlockData* Block = RailBlocks.Find(InBlockId))
	{
		if (!Block->bIsOccupied || Block->OccupyingTrainId == InTrainId)
		{
			Block->bIsOccupied = true;
			Block->OccupyingTrainId = InTrainId;
			Block->SignalState = ESBRailSignalState::StopRed;
			OnRailSignalChanged.Broadcast(InBlockId, Block->SignalState);
			return true;
		}
		return false;
	}
	return true; // Uncontrolled track is free
}

void USBRailNetworkComponent::ReleaseBlockReservation(int32 InBlockId, FName InTrainId)
{
	if (FSBRailBlockData* Block = RailBlocks.Find(InBlockId))
	{
		if (Block->OccupyingTrainId == InTrainId)
		{
			Block->bIsOccupied = false;
			Block->OccupyingTrainId = NAME_None;
			Block->SignalState = ESBRailSignalState::ClearGreen;
			OnRailSignalChanged.Broadcast(InBlockId, Block->SignalState);
		}
	}
}

int32 USBRailNetworkComponent::LoadCargoIntoWagon(int32 WagonIndex, FName ItemId, int32 Quantity)
{
	if (!TrainConsist.Wagons.IsValidIndex(WagonIndex) || ItemId.IsNone() || Quantity <= 0)
	{
		return 0;
	}

	FSBRailWagonData& Wagon = TrainConsist.Wagons[WagonIndex];
	int32 Current = Wagon.CargoInventory.FindRef(ItemId);
	int32 SpaceAvailable = Wagon.CargoCapacity - Current;
	int32 AmountToAdd = FMath::Clamp(Quantity, 0, SpaceAvailable);

	if (AmountToAdd > 0)
	{
		Wagon.CargoInventory.FindOrAdd(ItemId) += AmountToAdd;
		OnTrainCargoTransferred.Broadcast(TrainConsist.TrainId, ItemId, AmountToAdd);
	}

	return AmountToAdd;
}

int32 USBRailNetworkComponent::UnloadCargoFromWagon(int32 WagonIndex, FName ItemId, int32 Quantity)
{
	if (!TrainConsist.Wagons.IsValidIndex(WagonIndex) || ItemId.IsNone() || Quantity <= 0)
	{
		return 0;
	}

	FSBRailWagonData& Wagon = TrainConsist.Wagons[WagonIndex];
	int32 Current = Wagon.CargoInventory.FindRef(ItemId);
	int32 AmountToRemove = FMath::Clamp(Quantity, 0, Current);

	if (AmountToRemove > 0)
	{
		Wagon.CargoInventory.FindOrAdd(ItemId) -= AmountToRemove;
		OnTrainCargoTransferred.Broadcast(TrainConsist.TrainId, ItemId, -AmountToRemove);
	}

	return AmountToRemove;
}

int32 USBRailNetworkComponent::GetWagonCargoCount(int32 WagonIndex, FName ItemId) const
{
	if (TrainConsist.Wagons.IsValidIndex(WagonIndex))
	{
		return TrainConsist.Wagons[WagonIndex].CargoInventory.FindRef(ItemId);
	}
	return 0;
}

void USBRailNetworkComponent::SimulateRailTick(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		SyncTags();
		return;
	}

	switch (TrainConsist.MovementState)
	{
	case ESBTrainMovementState::Traveling:
	{
		int32 NextBlockId = TrainConsist.CurrentTrackSegmentId + 1;
		if (FSBRailBlockData* NextBlock = RailBlocks.Find(NextBlockId))
		{
			if (NextBlock->bIsOccupied && NextBlock->OccupyingTrainId != TrainConsist.TrainId)
			{
				// Approaching red signal -> decelerate and wait
				TrainConsist.CurrentSpeed = FMath::Max(0.0f, TrainConsist.CurrentSpeed - Settings.Deceleration * DeltaTime);
				if (TrainConsist.CurrentSpeed <= KINDA_SMALL_NUMBER)
				{
					SyncTrainState(ESBTrainMovementState::WaitingSignal);
					return;
				}
			}
		}

		// Accelerate to max speed
		TrainConsist.CurrentSpeed = FMath::Min(TrainConsist.MaxSpeed, TrainConsist.CurrentSpeed + Settings.Acceleration * DeltaTime);
		float DistanceCovered = (TrainConsist.CurrentSpeed / 3.6f) * DeltaTime; // km/h to m/s
		float SegmentLength = 100.0f; // 100 meters per segment
		TrainConsist.CurrentTrackProgressAlpha += DistanceCovered / SegmentLength;

		if (TrainConsist.CurrentTrackProgressAlpha >= 1.0f)
		{
			ReleaseBlockReservation(TrainConsist.CurrentTrackSegmentId, TrainConsist.TrainId);
			TrainConsist.CurrentTrackProgressAlpha -= 1.0f;
			TrainConsist.CurrentTrackSegmentId++;
			RequestBlockReservation(TrainConsist.CurrentTrackSegmentId, TrainConsist.TrainId);

			// Check station arrival
			if (TrainConsist.DestinationStations.IsValidIndex(TrainConsist.CurrentStationIndex))
			{
				FName TargetStation = TrainConsist.DestinationStations[TrainConsist.CurrentStationIndex];
				TrainConsist.CurrentSpeed = 0.0f;
				OnTrainStationArrived.Broadcast(TrainConsist.TrainId, TargetStation);

				// Determine if Loading or Unloading
				bool bHasCargo = false;
				for (const FSBRailWagonData& Wagon : TrainConsist.Wagons)
				{
					for (const auto& Pair : Wagon.CargoInventory)
					{
						if (Pair.Value > 0) { bHasCargo = true; break; }
					}
				}

				if (bHasCargo)
				{
					SyncTrainState(ESBTrainMovementState::Unloading);
				}
				else
				{
					SyncTrainState(ESBTrainMovementState::Loading);
				}
				TrainConsist.StationWaitTimer = 0.0f;
			}
		}
		break;
	}
	case ESBTrainMovementState::WaitingSignal:
	{
		int32 NextBlockId = TrainConsist.CurrentTrackSegmentId + 1;
		if (RequestBlockReservation(NextBlockId, TrainConsist.TrainId))
		{
			SyncTrainState(ESBTrainMovementState::Traveling);
		}
		break;
	}
	case ESBTrainMovementState::Loading:
	case ESBTrainMovementState::Unloading:
	{
		TrainConsist.StationWaitTimer += DeltaTime;
		if (TrainConsist.StationWaitTimer >= TrainConsist.MaxStationWaitDuration)
		{
			TrainConsist.StationWaitTimer = 0.0f;
			if (TrainConsist.DestinationStations.Num() > 0)
			{
				TrainConsist.CurrentStationIndex = (TrainConsist.CurrentStationIndex + 1) % TrainConsist.DestinationStations.Num();
			}
			SyncTrainState(ESBTrainMovementState::Traveling);
		}
		break;
	}
	default:
		break;
	}
}

void USBRailNetworkComponent::SyncTrainState(ESBTrainMovementState NewState)
{
	if (TrainConsist.MovementState != NewState)
	{
		TrainConsist.MovementState = NewState;
		OnTrainStateChanged.Broadcast(TrainConsist.TrainId, NewState);
		SyncTags();
	}
}

void USBRailNetworkComponent::SyncTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	CachedStateComp->RemoveTag(Tags.State_Rail_Traveling);
	CachedStateComp->RemoveTag(Tags.State_Rail_Loading);
	CachedStateComp->RemoveTag(Tags.State_Rail_Unloading);
	CachedStateComp->RemoveTag(Tags.State_Rail_WaitingSignal);
	CachedStateComp->RemoveTag(Tags.State_Rail_Derailed);

	switch (TrainConsist.MovementState)
	{
	case ESBTrainMovementState::Traveling:
		CachedStateComp->AddTag(Tags.State_Rail_Traveling);
		break;
	case ESBTrainMovementState::Loading:
		CachedStateComp->AddTag(Tags.State_Rail_Loading);
		break;
	case ESBTrainMovementState::Unloading:
		CachedStateComp->AddTag(Tags.State_Rail_Unloading);
		break;
	case ESBTrainMovementState::WaitingSignal:
		CachedStateComp->AddTag(Tags.State_Rail_WaitingSignal);
		break;
	case ESBTrainMovementState::Derailed:
		CachedStateComp->AddTag(Tags.State_Rail_Derailed);
		break;
	default:
		break;
	}
}
