// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBPowerGridComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBPowerGridComponent::USBPowerGridComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBPowerGridComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	TArray<USBPowerGridComponent*> Visited;
	SimulatePowerGridTick(0.0f, Visited);
}

void USBPowerGridComponent::OnShutdown_Implementation()
{
	TArray<TWeakObjectPtr<USBPowerGridComponent>> NodesCopy = ConnectedGridNodes;
	for (const TWeakObjectPtr<USBPowerGridComponent>& NodePtr : NodesCopy)
	{
		if (NodePtr.IsValid())
		{
			DisconnectFromPowerNode(NodePtr.Get());
		}
	}
}

void USBPowerGridComponent::SetupNode(ESBPowerNodeType Type, float Production, float Consumption, float BatteryCap)
{
	NodeData.NodeType = Type;
	NodeData.PowerGeneration = Production;
	NodeData.PowerConsumption = Consumption;
	NodeData.BatteryCapacity = BatteryCap;
	if (Type == ESBPowerNodeType::Battery)
	{
		NodeData.BatteryStoredEnergy = FMath::Min(NodeData.BatteryStoredEnergy, BatteryCap);
	}

	TArray<USBPowerGridComponent*> Visited;
	SimulatePowerGridTick(0.0f, Visited);
}

bool USBPowerGridComponent::ConnectToPowerNode(USBPowerGridComponent* TargetNode)
{
	if (!TargetNode || TargetNode == this || ConnectedGridNodes.Contains(TargetNode))
	{
		return false;
	}

	if (ConnectedGridNodes.Num() >= Settings.MaxWireConnections || TargetNode->ConnectedGridNodes.Num() >= TargetNode->Settings.MaxWireConnections)
	{
		return false;
	}

	ConnectedGridNodes.Add(TargetNode);
	if (!TargetNode->ConnectedGridNodes.Contains(this))
	{
		TargetNode->ConnectedGridNodes.Add(this);
	}

	TArray<USBPowerGridComponent*> Visited;
	SimulatePowerGridTick(0.0f, Visited);
	return true;
}

void USBPowerGridComponent::DisconnectFromPowerNode(USBPowerGridComponent* TargetNode)
{
	if (!TargetNode)
	{
		return;
	}

	ConnectedGridNodes.Remove(TargetNode);
	TargetNode->ConnectedGridNodes.Remove(this);

	TArray<USBPowerGridComponent*> Visited;
	SimulatePowerGridTick(0.0f, Visited);

	TArray<USBPowerGridComponent*> TargetVisited;
	TargetNode->SimulatePowerGridTick(0.0f, TargetVisited);
}

void USBPowerGridComponent::SetBreakerTripped(bool bTripped)
{
	NodeData.bIsBreakerTripped = bTripped;
	OnBreakerTripped.Broadcast(bTripped);

	TArray<USBPowerGridComponent*> Visited;
	SimulatePowerGridTick(0.0f, Visited);
}

void USBPowerGridComponent::GetConnectedSubnet(TArray<USBPowerGridComponent*>& OutSubnet)
{
	OutSubnet.Empty();
	TArray<USBPowerGridComponent*> Queue;
	Queue.Add(this);
	OutSubnet.Add(this);

	while (Queue.Num() > 0)
	{
		USBPowerGridComponent* Current = Queue[0];
		Queue.RemoveAt(0);

		for (const TWeakObjectPtr<USBPowerGridComponent>& NeighborPtr : Current->ConnectedGridNodes)
		{
			if (NeighborPtr.IsValid() && !OutSubnet.Contains(NeighborPtr.Get()))
			{
				OutSubnet.Add(NeighborPtr.Get());
				Queue.Add(NeighborPtr.Get());
			}
		}
	}
}

void USBPowerGridComponent::SimulatePowerGridTick(float DeltaTime, TArray<USBPowerGridComponent*>& Visited)
{
	if (Visited.Contains(this))
	{
		return;
	}

	TArray<USBPowerGridComponent*> Subnet;
	GetConnectedSubnet(Subnet);

	for (USBPowerGridComponent* Node : Subnet)
	{
		Visited.AddUnique(Node);
	}

	float TotalGen = 0.0f;
	float TotalDem = 0.0f;
	float TotalBatteryStored = 0.0f;
	float TotalBatteryCap = 0.0f;
	bool bAnyBreakerTripped = false;

	for (USBPowerGridComponent* Node : Subnet)
	{
		if (Node->NodeData.bIsBreakerTripped)
		{
			bAnyBreakerTripped = true;
		}

		TotalGen += Node->NodeData.PowerGeneration;
		TotalDem += Node->NodeData.PowerConsumption;

		if (Node->NodeData.NodeType == ESBPowerNodeType::Battery)
		{
			TotalBatteryStored += Node->NodeData.BatteryStoredEnergy;
			TotalBatteryCap += Node->NodeData.BatteryCapacity;
		}
	}

	if (bAnyBreakerTripped || (TotalGen > 0.0f && TotalDem > TotalGen * Settings.OverloadThreshold))
	{
		for (USBPowerGridComponent* Node : Subnet)
		{
			Node->NodeData.GridTotalProduction = TotalGen;
			Node->NodeData.GridTotalDemand = TotalDem;
			Node->NodeData.PowerSatisfactionRatio = 0.0f;
			Node->NodeData.GridState = ESBPowerGridState::Overloaded;

			Node->SyncPowerTags();
			Node->OnPowerGridStateChanged.Broadcast(Node->NodeData.GridState);
			Node->OnPowerFlowChanged.Broadcast(TotalGen, TotalDem);
		}
		return;
	}

	float NetPower = TotalGen - TotalDem;

	if (NetPower >= 0.0f)
	{
		float ChargeAmount = FMath::Min(NetPower, Settings.BatteryChargeRate * (DeltaTime > 0.0f ? DeltaTime : 1.0f));

		for (USBPowerGridComponent* Node : Subnet)
		{
			if (Node->NodeData.NodeType == ESBPowerNodeType::Battery && Node->NodeData.BatteryCapacity > 0.0f)
			{
				Node->NodeData.BatteryStoredEnergy = FMath::Min(Node->NodeData.BatteryCapacity, Node->NodeData.BatteryStoredEnergy + ChargeAmount);
			}
		}

		for (USBPowerGridComponent* Node : Subnet)
		{
			Node->NodeData.GridTotalProduction = TotalGen;
			Node->NodeData.GridTotalDemand = TotalDem;
			Node->NodeData.PowerSatisfactionRatio = 1.0f;

			if (Node->NodeData.NodeType == ESBPowerNodeType::Battery)
			{
				Node->NodeData.GridState = ESBPowerGridState::Charging;
			}
			else if (TotalGen > 0.0f || TotalDem > 0.0f)
			{
				Node->NodeData.GridState = ESBPowerGridState::Powered;
			}
			else
			{
				Node->NodeData.GridState = ESBPowerGridState::Unpowered;
			}

			Node->SyncPowerTags();
			Node->OnPowerGridStateChanged.Broadcast(Node->NodeData.GridState);
			Node->OnPowerFlowChanged.Broadcast(TotalGen, TotalDem);
		}
	}
	else
	{
		float Deficit = FMath::Abs(NetPower);
		if (TotalBatteryStored > 0.0f)
		{
			float Discharged = FMath::Min(TotalBatteryStored, (DeltaTime > 0.0f ? (Settings.BatteryDischargeRate * DeltaTime) : Deficit));

			for (USBPowerGridComponent* Node : Subnet)
			{
				if (Node->NodeData.NodeType == ESBPowerNodeType::Battery)
				{
					Node->NodeData.BatteryStoredEnergy = FMath::Max(0.0f, Node->NodeData.BatteryStoredEnergy - Discharged);
				}
			}

			for (USBPowerGridComponent* Node : Subnet)
			{
				Node->NodeData.GridTotalProduction = TotalGen;
				Node->NodeData.GridTotalDemand = TotalDem;
				Node->NodeData.PowerSatisfactionRatio = 1.0f;

				if (Node->NodeData.NodeType == ESBPowerNodeType::Battery)
				{
					Node->NodeData.GridState = ESBPowerGridState::Discharging;
				}
				else
				{
					Node->NodeData.GridState = ESBPowerGridState::Powered;
				}

				Node->SyncPowerTags();
				Node->OnPowerGridStateChanged.Broadcast(Node->NodeData.GridState);
				Node->OnPowerFlowChanged.Broadcast(TotalGen, TotalDem);
			}
		}
		else
		{
			for (USBPowerGridComponent* Node : Subnet)
			{
				Node->NodeData.GridTotalProduction = TotalGen;
				Node->NodeData.GridTotalDemand = TotalDem;
				Node->NodeData.PowerSatisfactionRatio = TotalDem > 0.0f ? (TotalGen / TotalDem) : 0.0f;
				Node->NodeData.GridState = ESBPowerGridState::Unpowered;

				Node->SyncPowerTags();
				Node->OnPowerGridStateChanged.Broadcast(Node->NodeData.GridState);
				Node->OnPowerFlowChanged.Broadcast(TotalGen, TotalDem);
			}
		}
	}
}

void USBPowerGridComponent::SyncPowerTags()
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

	CachedStateComp->RemoveTag(Tags.State_Power_Powered);
	CachedStateComp->RemoveTag(Tags.State_Power_Unpowered);
	CachedStateComp->RemoveTag(Tags.State_Power_Overloaded);
	CachedStateComp->RemoveTag(Tags.State_Power_Charging);
	CachedStateComp->RemoveTag(Tags.State_Power_Discharging);

	if (NodeData.GridState == ESBPowerGridState::Powered)
	{
		CachedStateComp->AddTag(Tags.State_Power_Powered);
	}
	else if (NodeData.GridState == ESBPowerGridState::Charging)
	{
		CachedStateComp->AddTag(Tags.State_Power_Charging);
	}
	else if (NodeData.GridState == ESBPowerGridState::Discharging)
	{
		CachedStateComp->AddTag(Tags.State_Power_Discharging);
	}
	else if (NodeData.GridState == ESBPowerGridState::Overloaded)
	{
		CachedStateComp->AddTag(Tags.State_Power_Overloaded);
	}
	else
	{
		CachedStateComp->AddTag(Tags.State_Power_Unpowered);
	}
}
