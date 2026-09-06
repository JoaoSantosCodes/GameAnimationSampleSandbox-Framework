#include "Components/SBCargoDroneNetworkComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBCargoDroneNetworkComponent::USBCargoDroneNetworkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBCargoDroneNetworkComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBCargoDroneNetworkComponent::SetupDrone(FName InDroneId, ESBCargoDroneModel InModel, float InFlightSpeed, float InMaxBattery, int32 InCapacity)
{
	DroneData.DroneId = InDroneId;
	DroneData.DroneModel = InModel;
	DroneData.FlightSpeed = InFlightSpeed;
	DroneData.MaxBattery = InMaxBattery;
	DroneData.CurrentBattery = InMaxBattery;
	DroneData.CargoBayCapacity = InCapacity;
	DroneData.FlightState = ESBCargoDroneFlightState::IdleAtPort;
	DroneData.FlightProgressAlpha = 0.0f;
	DroneData.CargoBay.Empty();
	SyncTags();
}

void USBCargoDroneNetworkComponent::RegisterDronePort(FName PortId, const FVector& Location, bool bHasRecharge)
{
	if (PortId.IsNone()) return;

	FSBDronePortData PortData;
	PortData.PortId = PortId;
	PortData.PortLocation = Location;
	PortData.bHasRechargeDock = bHasRecharge;
	RegisteredPorts.Add(PortId, PortData);

	if (DroneData.HomePortId.IsNone())
	{
		DroneData.HomePortId = PortId;
		DroneData.HomePortLocation = Location;
		DroneData.CurrentLocation = Location;
	}
}

void USBCargoDroneNetworkComponent::SetFlightRoute(FName OriginPortId, FName DestinationPortId)
{
	DroneData.HomePortId = OriginPortId;
	if (const FSBDronePortData* OriginPort = RegisteredPorts.Find(OriginPortId))
	{
		DroneData.HomePortLocation = OriginPort->PortLocation;
		DroneData.CurrentLocation = OriginPort->PortLocation;
	}

	DroneData.TargetPortId = DestinationPortId;
	if (const FSBDronePortData* DestPort = RegisteredPorts.Find(DestinationPortId))
	{
		DroneData.TargetPortLocation = DestPort->PortLocation;
	}
}

bool USBCargoDroneNetworkComponent::DispatchDrone()
{
	if (DroneData.FlightState != ESBCargoDroneFlightState::IdleAtPort)
	{
		return false;
	}

	if (DroneData.TargetPortId.IsNone())
	{
		return false;
	}

	DroneData.FlightState = ESBCargoDroneFlightState::TakingOff;
	DroneData.FlightProgressAlpha = 0.0f;
	StateTimer = 0.0f;
	OnDroneFlightStateChanged.Broadcast(DroneData.DroneId, DroneData.FlightState);
	SyncTags();
	return true;
}

int32 USBCargoDroneNetworkComponent::LoadCargoIntoDrone(FName ItemId, int32 Quantity)
{
	if (ItemId.IsNone() || Quantity <= 0) return 0;

	int32 CurrentTotal = GetTotalCargoCount();
	int32 SpaceAvailable = FMath::Max(0, DroneData.CargoBayCapacity - CurrentTotal);
	int32 Added = FMath::Min(Quantity, SpaceAvailable);

	if (Added > 0)
	{
		DroneData.CargoBay.FindOrAdd(ItemId) += Added;
		OnDroneCargoLoaded.Broadcast(DroneData.DroneId, ItemId, Added);
	}

	return Added;
}

int32 USBCargoDroneNetworkComponent::UnloadCargoFromDrone(FName ItemId, int32 Quantity)
{
	if (ItemId.IsNone() || Quantity <= 0) return 0;

	int32* CurrentStock = DroneData.CargoBay.Find(ItemId);
	if (!CurrentStock || *CurrentStock <= 0) return 0;

	int32 Removed = FMath::Min(Quantity, *CurrentStock);
	*CurrentStock -= Removed;

	if (*CurrentStock <= 0)
	{
		DroneData.CargoBay.Remove(ItemId);
	}

	return Removed;
}

int32 USBCargoDroneNetworkComponent::GetDroneCargoCount(FName ItemId) const
{
	if (ItemId.IsNone()) return 0;
	return DroneData.CargoBay.FindRef(ItemId);
}

int32 USBCargoDroneNetworkComponent::GetTotalCargoCount() const
{
	int32 Total = 0;
	for (const auto& Pair : DroneData.CargoBay)
	{
		Total += Pair.Value;
	}
	return Total;
}

void USBCargoDroneNetworkComponent::SimulateDroneTick(float DeltaTime)
{
	switch (DroneData.FlightState)
	{
	case ESBCargoDroneFlightState::IdleAtPort:
	{
		const FSBDronePortData* Port = RegisteredPorts.Find(DroneData.HomePortId);
		if (Port && Port->bHasRechargeDock && DroneData.CurrentBattery < DroneData.MaxBattery)
		{
			DroneData.FlightState = ESBCargoDroneFlightState::Recharging;
			OnDroneFlightStateChanged.Broadcast(DroneData.DroneId, DroneData.FlightState);
		}
		break;
	}
	case ESBCargoDroneFlightState::Recharging:
	{
		DroneData.CurrentBattery = FMath::Min(DroneData.MaxBattery, DroneData.CurrentBattery + DroneData.BatteryRechargeRate * DeltaTime);
		OnDroneBatteryUpdated.Broadcast(DroneData.DroneId, DroneData.CurrentBattery);

		if (DroneData.CurrentBattery >= DroneData.MaxBattery)
		{
			DroneData.FlightState = ESBCargoDroneFlightState::IdleAtPort;
			OnDroneFlightStateChanged.Broadcast(DroneData.DroneId, DroneData.FlightState);
		}
		break;
	}
	case ESBCargoDroneFlightState::TakingOff:
	{
		StateTimer += DeltaTime;
		DroneData.CurrentBattery = FMath::Max(0.0f, DroneData.CurrentBattery - DroneData.BatteryDischargeRate * DeltaTime);
		OnDroneBatteryUpdated.Broadcast(DroneData.DroneId, DroneData.CurrentBattery);

		if (StateTimer >= Settings.TakeoffDuration)
		{
			StateTimer = 0.0f;
			DroneData.FlightState = ESBCargoDroneFlightState::InFlight;
			OnDroneFlightStateChanged.Broadcast(DroneData.DroneId, DroneData.FlightState);
		}
		break;
	}
	case ESBCargoDroneFlightState::InFlight:
	case ESBCargoDroneFlightState::LowBatteryReturn:
	{
		DroneData.CurrentBattery = FMath::Max(0.0f, DroneData.CurrentBattery - DroneData.BatteryDischargeRate * DeltaTime);
		OnDroneBatteryUpdated.Broadcast(DroneData.DroneId, DroneData.CurrentBattery);

		if (DroneData.FlightState == ESBCargoDroneFlightState::InFlight && DroneData.CurrentBattery <= DroneData.LowBatteryThreshold)
		{
			DroneData.FlightState = ESBCargoDroneFlightState::LowBatteryReturn;
			DroneData.TargetPortId = DroneData.HomePortId;
			DroneData.TargetPortLocation = DroneData.HomePortLocation;

			// O desvio reinicia a rota: o drone passa a voar de volta para a base. Sem zerar
			// o progresso e sair do tick, o avanco logo abaixo completava a rota ORIGINAL no
			// mesmo tick e sobrescrevia o estado de emergencia por Landing -- o drone pousava
			// no destino em vez de retornar.
			DroneData.FlightProgressAlpha = 0.0f;
			OnDroneFlightStateChanged.Broadcast(DroneData.DroneId, DroneData.FlightState);
			break;
		}

		// Advance progress assuming standard corridor speed
		float ProgressRate = (DroneData.FlightSpeed > 0.0f) ? (DroneData.FlightSpeed / 60.0f * 0.1f) : 0.1f;
		DroneData.FlightProgressAlpha = FMath::Clamp(DroneData.FlightProgressAlpha + ProgressRate * DeltaTime, 0.0f, 1.0f);

		if (DroneData.FlightProgressAlpha >= 1.0f)
		{
			DroneData.FlightState = ESBCargoDroneFlightState::Landing;
			StateTimer = 0.0f;
			OnDroneFlightStateChanged.Broadcast(DroneData.DroneId, DroneData.FlightState);
		}
		break;
	}
	case ESBCargoDroneFlightState::Landing:
	{
		StateTimer += DeltaTime;
		if (StateTimer >= Settings.LandingDuration)
		{
			StateTimer = 0.0f;
			DroneData.FlightState = ESBCargoDroneFlightState::IdleAtPort;
			DroneData.TotalTripsCompleted++;
			DroneData.HomePortId = DroneData.TargetPortId;
			DroneData.HomePortLocation = DroneData.TargetPortLocation;
			DroneData.CurrentLocation = DroneData.TargetPortLocation;
			DroneData.FlightProgressAlpha = 0.0f;
			OnDroneFlightStateChanged.Broadcast(DroneData.DroneId, DroneData.FlightState);
			OnDronePortArrived.Broadcast(DroneData.DroneId, DroneData.HomePortId);
		}
		break;
	}
	}

	SyncTags();
}

void USBCargoDroneNetworkComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Drone_Idle);
	CachedStateComp->RemoveTag(Tags.State_Drone_TakingOff);
	CachedStateComp->RemoveTag(Tags.State_Drone_InFlight);
	CachedStateComp->RemoveTag(Tags.State_Drone_Landing);
	CachedStateComp->RemoveTag(Tags.State_Drone_Recharging);
	CachedStateComp->RemoveTag(Tags.State_Drone_LowBattery);

	switch (DroneData.FlightState)
	{
	case ESBCargoDroneFlightState::IdleAtPort:
		CachedStateComp->AddTag(Tags.State_Drone_Idle);
		break;
	case ESBCargoDroneFlightState::TakingOff:
		CachedStateComp->AddTag(Tags.State_Drone_TakingOff);
		break;
	case ESBCargoDroneFlightState::InFlight:
		CachedStateComp->AddTag(Tags.State_Drone_InFlight);
		break;
	case ESBCargoDroneFlightState::Landing:
		CachedStateComp->AddTag(Tags.State_Drone_Landing);
		break;
	case ESBCargoDroneFlightState::Recharging:
		CachedStateComp->AddTag(Tags.State_Drone_Recharging);
		break;
	case ESBCargoDroneFlightState::LowBatteryReturn:
		CachedStateComp->AddTag(Tags.State_Drone_LowBattery);
		break;
	}
}
