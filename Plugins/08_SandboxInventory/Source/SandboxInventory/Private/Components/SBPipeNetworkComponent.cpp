#include "Components/SBPipeNetworkComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBPipeNetworkComponent::USBPipeNetworkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBPipeNetworkComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	TArray<USBPipeNetworkComponent*> Visited;
	SimulateFluidDynamicsTick(0.0f, Visited);
}

void USBPipeNetworkComponent::OnShutdown_Implementation()
{
	TArray<TWeakObjectPtr<USBPipeNetworkComponent>> PipesCopy = ConnectedPipes;
	for (const TWeakObjectPtr<USBPipeNetworkComponent>& PipePtr : PipesCopy)
	{
		if (PipePtr.IsValid())
		{
			DisconnectPipe(PipePtr.Get());
		}
	}
}

void USBPipeNetworkComponent::SetupNode(ESBPipeNodeType InNodeType, ESBFluidType InFluidType, float InCapacity, float InMaxPressure)
{
	NodeData.NodeType = InNodeType;
	NodeData.FluidType = InFluidType;
	NodeData.FluidCapacity = InCapacity;
	NodeData.MaxSafePressure = InMaxPressure;

	TArray<USBPipeNetworkComponent*> Visited;
	SimulateFluidDynamicsTick(0.0f, Visited);
}

void USBPipeNetworkComponent::SetPumpActive(bool bActive)
{
	NodeData.bIsPumpActive = bActive;
	TArray<USBPipeNetworkComponent*> Visited;
	SimulateFluidDynamicsTick(0.0f, Visited);
}

void USBPipeNetworkComponent::SetValveOpenPercentage(float InOpenPct)
{
	NodeData.ValveOpenPercentage = FMath::Clamp(InOpenPct, 0.0f, 1.0f);
	TArray<USBPipeNetworkComponent*> Visited;
	SimulateFluidDynamicsTick(0.0f, Visited);
}

bool USBPipeNetworkComponent::ConnectPipe(USBPipeNetworkComponent* TargetPipe)
{
	if (!TargetPipe || TargetPipe == this || ConnectedPipes.Contains(TargetPipe))
	{
		return false;
	}

	if (ConnectedPipes.Num() >= Settings.MaxPipeConnections || TargetPipe->ConnectedPipes.Num() >= TargetPipe->Settings.MaxPipeConnections)
	{
		return false;
	}

	ConnectedPipes.Add(TargetPipe);
	if (!TargetPipe->ConnectedPipes.Contains(this))
	{
		TargetPipe->ConnectedPipes.Add(this);
	}

	TArray<USBPipeNetworkComponent*> Visited;
	SimulateFluidDynamicsTick(0.0f, Visited);
	return true;
}

void USBPipeNetworkComponent::DisconnectPipe(USBPipeNetworkComponent* TargetPipe)
{
	if (!TargetPipe)
	{
		return;
	}

	ConnectedPipes.Remove(TargetPipe);
	TargetPipe->ConnectedPipes.Remove(this);

	TArray<USBPipeNetworkComponent*> Visited;
	SimulateFluidDynamicsTick(0.0f, Visited);

	TArray<USBPipeNetworkComponent*> TargetVisited;
	TargetPipe->SimulateFluidDynamicsTick(0.0f, TargetVisited);
}

float USBPipeNetworkComponent::InjectFluid(ESBFluidType InType, float InAmount)
{
	if (InAmount <= 0.0f || NodeData.bIsRuptured)
	{
		return 0.0f;
	}

	if (NodeData.FluidType != ESBFluidType::None && NodeData.FluidType != InType && NodeData.FluidAmount > 0.0f)
	{
		return 0.0f;
	}

	NodeData.FluidType = InType;
	float AvailableSpace = FMath::Max(0.0f, NodeData.FluidCapacity - NodeData.FluidAmount);
	float Accepted = FMath::Min(AvailableSpace, InAmount);
	NodeData.FluidAmount += Accepted;

	OnFluidLevelChanged.Broadcast(NodeData.FluidAmount, NodeData.FluidCapacity);

	TArray<USBPipeNetworkComponent*> Visited;
	SimulateFluidDynamicsTick(0.0f, Visited);
	return Accepted;
}

float USBPipeNetworkComponent::ExtractFluid(float InAmount)
{
	if (InAmount <= 0.0f || NodeData.FluidAmount <= 0.0f)
	{
		return 0.0f;
	}

	float Extracted = FMath::Min(NodeData.FluidAmount, InAmount);
	NodeData.FluidAmount -= Extracted;
	if (NodeData.FluidAmount <= 0.0f)
	{
		NodeData.FluidType = ESBFluidType::None;
	}

	OnFluidLevelChanged.Broadcast(NodeData.FluidAmount, NodeData.FluidCapacity);

	TArray<USBPipeNetworkComponent*> Visited;
	SimulateFluidDynamicsTick(0.0f, Visited);
	return Extracted;
}

void USBPipeNetworkComponent::GetConnectedPipeNetwork(TArray<USBPipeNetworkComponent*>& OutNetwork)
{
	OutNetwork.Empty();
	TArray<USBPipeNetworkComponent*> Queue;
	Queue.Add(this);
	OutNetwork.Add(this);

	while (Queue.Num() > 0)
	{
		USBPipeNetworkComponent* Current = Queue[0];
		Queue.RemoveAt(0);

		for (const TWeakObjectPtr<USBPipeNetworkComponent>& NeighborPtr : Current->ConnectedPipes)
		{
			if (NeighborPtr.IsValid() && !OutNetwork.Contains(NeighborPtr.Get()))
			{
				OutNetwork.Add(NeighborPtr.Get());
				Queue.Add(NeighborPtr.Get());
			}
		}
	}
}

void USBPipeNetworkComponent::SimulateFluidDynamicsTick(float DeltaTime, TArray<USBPipeNetworkComponent*>& Visited)
{
	if (Visited.Contains(this))
	{
		return;
	}

	TArray<USBPipeNetworkComponent*> Network;
	GetConnectedPipeNetwork(Network);

	for (USBPipeNetworkComponent* Pipe : Network)
	{
		Visited.AddUnique(Pipe);
	}

	float MaxPumpPressure = 0.0f;
	ESBFluidType NetworkFluid = ESBFluidType::None;

	for (USBPipeNetworkComponent* Pipe : Network)
	{
		if (Pipe->NodeData.bIsPumpActive)
		{
			MaxPumpPressure = FMath::Max(MaxPumpPressure, Pipe->Settings.PumpPressureGeneration);
		}
		if (Pipe->NodeData.FluidType != ESBFluidType::None)
		{
			NetworkFluid = Pipe->NodeData.FluidType;
		}
	}

	for (USBPipeNetworkComponent* Pipe : Network)
	{
		if (Pipe->NodeData.bIsRuptured)
		{
			Pipe->NodeData.FlowState = ESBPipeFlowState::Ruptured;
			Pipe->NodeData.CurrentPressure = 0.0f;
			Pipe->SyncFluidTags();
			Pipe->OnPipeFlowStateChanged.Broadcast(Pipe->NodeData.FlowState);
			continue;
		}

		if (Pipe->NodeData.bIsPumpActive)
		{
			Pipe->NodeData.CurrentPressure = Pipe->Settings.PumpPressureGeneration;
		}
		else
		{
			Pipe->NodeData.CurrentPressure = MaxPumpPressure;
		}

		if (Pipe->NodeData.CurrentPressure >= Pipe->NodeData.MaxSafePressure * Pipe->Settings.RuptureThresholdMultiplier)
		{
			Pipe->NodeData.bIsRuptured = true;
			Pipe->NodeData.FlowState = ESBPipeFlowState::Ruptured;
			float Lost = Pipe->NodeData.FluidAmount;
			Pipe->NodeData.FluidAmount = 0.0f;
			Pipe->SyncFluidTags();
			Pipe->OnPipeRupture.Broadcast(Pipe->NodeData.FluidType, Lost);
			Pipe->OnPipeFlowStateChanged.Broadcast(Pipe->NodeData.FlowState);
			continue;
		}

		if (Pipe->NodeData.NodeType == ESBPipeNodeType::Valve && Pipe->NodeData.ValveOpenPercentage <= 0.0f)
		{
			Pipe->NodeData.FlowState = ESBPipeFlowState::Blocked;
			Pipe->NodeData.FlowRate = 0.0f;
			Pipe->SyncFluidTags();
			Pipe->OnPipeFlowStateChanged.Broadcast(Pipe->NodeData.FlowState);
			continue;
		}

		if (Pipe->NodeData.CurrentPressure > 0.0f)
		{
			Pipe->NodeData.FlowState = ESBPipeFlowState::Flowing;
			Pipe->NodeData.FlowRate = Pipe->Settings.BaseFlowSpeed * Pipe->NodeData.ValveOpenPercentage;
			if (Pipe->NodeData.FluidType == ESBFluidType::None && NetworkFluid != ESBFluidType::None)
			{
				Pipe->NodeData.FluidType = NetworkFluid;
			}
		}
		else if (Pipe->NodeData.FluidAmount > 0.0f)
		{
			Pipe->NodeData.FlowState = ESBPipeFlowState::Pressurized;
			Pipe->NodeData.FlowRate = 0.0f;
		}
		else
		{
			Pipe->NodeData.FlowState = ESBPipeFlowState::Empty;
			Pipe->NodeData.FlowRate = 0.0f;
		}

		Pipe->SyncFluidTags();
		Pipe->OnPipeFlowStateChanged.Broadcast(Pipe->NodeData.FlowState);
		Pipe->OnFluidPressureChanged.Broadcast(Pipe->NodeData.CurrentPressure, Pipe->NodeData.MaxSafePressure);
	}
}

void USBPipeNetworkComponent::SyncFluidTags()
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

	CachedStateComp->RemoveTag(Tags.State_Fluid_Flowing);
	CachedStateComp->RemoveTag(Tags.State_Fluid_Blocked);
	CachedStateComp->RemoveTag(Tags.State_Fluid_Pressurized);
	CachedStateComp->RemoveTag(Tags.State_Fluid_Leaking);
	CachedStateComp->RemoveTag(Tags.State_Fluid_Ruptured);

	if (NodeData.FlowState == ESBPipeFlowState::Flowing)
	{
		CachedStateComp->AddTag(Tags.State_Fluid_Flowing);
		if (NodeData.CurrentPressure > 0.0f)
		{
			CachedStateComp->AddTag(Tags.State_Fluid_Pressurized);
		}
	}
	else if (NodeData.FlowState == ESBPipeFlowState::Blocked)
	{
		CachedStateComp->AddTag(Tags.State_Fluid_Blocked);
	}
	else if (NodeData.FlowState == ESBPipeFlowState::Pressurized)
	{
		CachedStateComp->AddTag(Tags.State_Fluid_Pressurized);
	}
	else if (NodeData.FlowState == ESBPipeFlowState::Leaking)
	{
		CachedStateComp->AddTag(Tags.State_Fluid_Leaking);
	}
	else if (NodeData.FlowState == ESBPipeFlowState::Ruptured)
	{
		CachedStateComp->AddTag(Tags.State_Fluid_Ruptured);
	}
}
