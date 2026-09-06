#include "Components/SBConveyorNetworkComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBConveyorNetworkComponent::USBConveyorNetworkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBConveyorNetworkComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncConveyorState();
}

void USBConveyorNetworkComponent::OnShutdown_Implementation()
{
	TArray<TWeakObjectPtr<USBConveyorNetworkComponent>> InputsCopy = InputConnections;
	for (const TWeakObjectPtr<USBConveyorNetworkComponent>& InPtr : InputsCopy)
	{
		if (InPtr.IsValid())
		{
			DisconnectInput(InPtr.Get());
		}
	}

	TArray<TWeakObjectPtr<USBConveyorNetworkComponent>> OutputsCopy = OutputConnections;
	for (const TWeakObjectPtr<USBConveyorNetworkComponent>& OutPtr : OutputsCopy)
	{
		if (OutPtr.IsValid())
		{
			DisconnectOutput(OutPtr.Get());
		}
	}
}

void USBConveyorNetworkComponent::SetupNode(ESBConveyorNodeType InNodeType, float InBeltSpeed, int32 InCapacity)
{
	NodeData.NodeType = InNodeType;
	NodeData.BeltSpeed = InBeltSpeed;
	NodeData.MaxItemCapacity = InCapacity;
	SyncConveyorState();
}

void USBConveyorNetworkComponent::SetFilterTag(FGameplayTag InTag)
{
	NodeData.FilterTag = InTag;
}

bool USBConveyorNetworkComponent::ConnectOutput(USBConveyorNetworkComponent* TargetNode)
{
	if (!TargetNode || TargetNode == this || OutputConnections.Contains(TargetNode))
	{
		return false;
	}

	if (OutputConnections.Num() >= Settings.MaxOutputConnections || TargetNode->InputConnections.Num() >= TargetNode->Settings.MaxInputConnections)
	{
		return false;
	}

	OutputConnections.Add(TargetNode);
	if (!TargetNode->InputConnections.Contains(this))
	{
		TargetNode->InputConnections.Add(this);
	}

	SyncConveyorState();
	TargetNode->SyncConveyorState();
	return true;
}

bool USBConveyorNetworkComponent::ConnectInput(USBConveyorNetworkComponent* SourceNode)
{
	if (!SourceNode)
	{
		return false;
	}

	return SourceNode->ConnectOutput(this);
}

void USBConveyorNetworkComponent::DisconnectOutput(USBConveyorNetworkComponent* TargetNode)
{
	if (!TargetNode)
	{
		return;
	}

	OutputConnections.Remove(TargetNode);
	TargetNode->InputConnections.Remove(this);

	SyncConveyorState();
	TargetNode->SyncConveyorState();
}

void USBConveyorNetworkComponent::DisconnectInput(USBConveyorNetworkComponent* SourceNode)
{
	if (!SourceNode)
	{
		return;
	}

	SourceNode->DisconnectOutput(this);
}

bool USBConveyorNetworkComponent::EnqueueItem(FName InItemId, int32 InQuantity, FGameplayTag InTag)
{
	if (InItemId.IsNone() || InQuantity <= 0 || ItemQueue.Num() >= NodeData.MaxItemCapacity)
	{
		return false;
	}

	FSBConveyorItemSlot Slot;
	Slot.ItemId = InItemId;
	Slot.Quantity = InQuantity;
	Slot.ItemCategoryTag = InTag;
	Slot.BeltProgressAlpha = 0.0f;

	ItemQueue.Add(Slot);
	NodeData.CurrentItemCount = ItemQueue.Num();
	SyncConveyorState();
	return true;
}

bool USBConveyorNetworkComponent::DequeueItem(FSBConveyorItemSlot& OutItem)
{
	if (ItemQueue.Num() == 0)
	{
		return false;
	}

	OutItem = ItemQueue[0];
	ItemQueue.RemoveAt(0);
	NodeData.CurrentItemCount = ItemQueue.Num();
	SyncConveyorState();
	return true;
}

bool USBConveyorNetworkComponent::PeekNextItem(FSBConveyorItemSlot& OutItem) const
{
	if (ItemQueue.Num() == 0)
	{
		return false;
	}

	OutItem = ItemQueue[0];
	return true;
}

void USBConveyorNetworkComponent::SimulateConveyorTick(float DeltaTime)
{
	if (ItemQueue.Num() == 0)
	{
		NodeData.bIsJammed = false;
		SyncConveyorState();
		return;
	}

	float ProgressDelta = (NodeData.BeltLength > 0.0f) ? (NodeData.BeltSpeed / NodeData.BeltLength) * DeltaTime : 1.0f;

	for (int32 i = 0; i < ItemQueue.Num(); ++i)
	{
		ItemQueue[i].BeltProgressAlpha = FMath::Clamp(ItemQueue[i].BeltProgressAlpha + ProgressDelta, 0.0f, 1.0f);
	}

	if (ItemQueue[0].BeltProgressAlpha >= 1.0f)
	{
		if (OutputConnections.Num() > 0)
		{
			USBConveyorNetworkComponent* TargetOutput = nullptr;

			if (NodeData.NodeType == ESBConveyorNodeType::SmartSorter)
			{
				bool bMatches = NodeData.FilterTag.IsValid() && ItemQueue[0].ItemCategoryTag.MatchesTag(NodeData.FilterTag);
				if (bMatches)
				{
					TargetOutput = OutputConnections[0].Get();
					OnConveyorItemFiltered.Broadcast(ItemQueue[0].ItemId, NodeData.FilterTag);
				}
				else if (OutputConnections.Num() > 1)
				{
					TargetOutput = OutputConnections[1].Get();
				}
				else
				{
					TargetOutput = OutputConnections[0].Get();
				}
			}
			else if (NodeData.NodeType == ESBConveyorNodeType::Splitter)
			{
				int32 ValidIndex = NodeData.SplitterRoundRobinIndex % OutputConnections.Num();
				TargetOutput = OutputConnections[ValidIndex].Get();
			}
			else
			{
				TargetOutput = OutputConnections[0].Get();
			}

			if (TargetOutput && TargetOutput->ItemQueue.Num() < TargetOutput->NodeData.MaxItemCapacity)
			{
				FSBConveyorItemSlot PoppedItem;
				DequeueItem(PoppedItem);
				TargetOutput->EnqueueItem(PoppedItem.ItemId, PoppedItem.Quantity, PoppedItem.ItemCategoryTag);
				OnConveyorItemTransferred.Broadcast(PoppedItem.ItemId, PoppedItem.Quantity);

				if (NodeData.NodeType == ESBConveyorNodeType::Splitter)
				{
					NodeData.SplitterRoundRobinIndex = (NodeData.SplitterRoundRobinIndex + 1) % OutputConnections.Num();
				}

				NodeData.bIsJammed = false;
			}
			else
			{
				NodeData.bIsJammed = true;
			}
		}
		else
		{
			NodeData.bIsJammed = true;
		}
	}
	else
	{
		NodeData.bIsJammed = false;
	}

	SyncConveyorState();
}

void USBConveyorNetworkComponent::SyncConveyorState()
{
	if (NodeData.bIsJammed)
	{
		NodeData.ConveyorState = ESBConveyorState::Jammed;
	}
	else if (NodeData.NodeType == ESBConveyorNodeType::Merger && InputConnections.Num() > 1)
	{
		NodeData.ConveyorState = ESBConveyorState::Merging;
	}
	else if (NodeData.NodeType == ESBConveyorNodeType::SmartSorter)
	{
		NodeData.ConveyorState = ESBConveyorState::Sorting;
	}
	else if (ItemQueue.Num() > 0)
	{
		NodeData.ConveyorState = ESBConveyorState::Conveying;
	}
	else
	{
		NodeData.ConveyorState = ESBConveyorState::Idle;
	}

	OnConveyorStateChanged.Broadcast(NodeData.ConveyorState);
	if (NodeData.bIsJammed)
	{
		OnConveyorJammed.Broadcast(true);
	}

	SyncTags();
}

void USBConveyorNetworkComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Logistics_Conveying);
	CachedStateComp->RemoveTag(Tags.State_Logistics_Jammed);
	CachedStateComp->RemoveTag(Tags.State_Logistics_Sorting);
	CachedStateComp->RemoveTag(Tags.State_Logistics_Merging);

	if (NodeData.ConveyorState == ESBConveyorState::Conveying)
	{
		CachedStateComp->AddTag(Tags.State_Logistics_Conveying);
	}
	else if (NodeData.ConveyorState == ESBConveyorState::Jammed)
	{
		CachedStateComp->AddTag(Tags.State_Logistics_Jammed);
	}
	else if (NodeData.ConveyorState == ESBConveyorState::Sorting)
	{
		CachedStateComp->AddTag(Tags.State_Logistics_Sorting);
	}
	else if (NodeData.ConveyorState == ESBConveyorState::Merging)
	{
		CachedStateComp->AddTag(Tags.State_Logistics_Merging);
	}
}
