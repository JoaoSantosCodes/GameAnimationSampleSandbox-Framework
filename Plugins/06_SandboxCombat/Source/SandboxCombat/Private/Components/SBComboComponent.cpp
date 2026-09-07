// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBComboComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBComboComponent::USBComboComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBComboComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBComboComponent::OnShutdown_Implementation()
{
	ResetCombo();
}

void USBComboComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentNodeId != INDEX_NONE)
	{
		WindowTimer -= DeltaTime;
		if (WindowTimer <= 0.0f)
		{
			ResetCombo();
		}
	}
}

void USBComboComponent::RegisterComboTree(const FSBComboTree& Tree)
{
	ActiveTree = Tree;
	ResetCombo();
}

bool USBComboComponent::ProcessComboInput(ESBComboInputType InputType)
{
	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	// 1. Caso Combo não iniciado: Procura nó inicial compatível
	if (CurrentNodeId == INDEX_NONE)
	{
		const FSBComboNode* StartNode = ActiveTree.Nodes.FindByPredicate([InputType](const FSBComboNode& Node)
		{
			return Node.ExpectedInput == InputType && (Node.NodeId == 0 || Node.NodeId == 100);
		});

		// Se não encontrou pelo ID padrão, pega o primeiro nó que espera este input
		if (!StartNode)
		{
			StartNode = ActiveTree.Nodes.FindByPredicate([InputType](const FSBComboNode& Node)
			{
				return Node.ExpectedInput == InputType;
			});
		}

		if (StartNode)
		{
			CurrentNodeId = StartNode->NodeId;
			ComboCounter = 1;
			WindowTimer = ActiveTree.MaxWindowDuration;
			bIsWindowOpen = false;
			bHasBufferedInput = false;

			UpdateFinisherTag(StartNode->bIsFinisher);
			OnComboStepExecuted.Broadcast(StartNode->NodeId, *StartNode, StartNode->DamageMultiplier);

			if (StartNode->bIsFinisher)
			{
				OnComboFinished.Broadcast(ComboCounter, StartNode->DamageMultiplier);
				ResetCombo();
			}
			return true;
		}
		return false;
	}

	// 2. Combo em andamento
	if (bIsWindowOpen)
	{
		const FSBComboNode* CurrentNode = ActiveTree.Nodes.FindByPredicate([this](const FSBComboNode& Node)
		{
			return Node.NodeId == CurrentNodeId;
		});

		if (!CurrentNode)
		{
			ResetCombo();
			return false;
		}

		// Procura nó de ramificação compatível entre os BranchTargetNodeIds
		const FSBComboNode* NextNode = nullptr;
		for (int32 BranchId : CurrentNode->BranchTargetNodeIds)
		{
			const FSBComboNode* BranchNode = ActiveTree.Nodes.FindByPredicate([BranchId](const FSBComboNode& Node)
			{
				return Node.NodeId == BranchId;
			});

			if (BranchNode && BranchNode->ExpectedInput == InputType)
			{
				NextNode = BranchNode;
				break;
			}
		}

		if (NextNode)
		{
			CurrentNodeId = NextNode->NodeId;
			ComboCounter++;
			WindowTimer = ActiveTree.MaxWindowDuration;
			CloseComboWindow();
			bHasBufferedInput = false;

			UpdateFinisherTag(NextNode->bIsFinisher);
			OnComboStepExecuted.Broadcast(NextNode->NodeId, *NextNode, NextNode->DamageMultiplier);

			if (NextNode->bIsFinisher)
			{
				OnComboFinished.Broadcast(ComboCounter, NextNode->DamageMultiplier);
				ResetCombo();
			}
			return true;
		}
		return false;
	}

	// 3. Janela fechada: Bufferiza input
	bHasBufferedInput = true;
	BufferedInputType = InputType;
	return true;
}

void USBComboComponent::OpenComboWindow()
{
	bIsWindowOpen = true;

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(FSBGameplayTags::Get().State_Combat_ComboWindowOpen);
	}

	// Consome input bufferizado se houver
	if (bHasBufferedInput)
	{
		ESBComboInputType InputToProcess = BufferedInputType;
		bHasBufferedInput = false;
		ProcessComboInput(InputToProcess);
	}
}

void USBComboComponent::CloseComboWindow()
{
	bIsWindowOpen = false;

	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(FSBGameplayTags::Get().State_Combat_ComboWindowOpen);
	}
}

void USBComboComponent::ResetCombo()
{
	CloseComboWindow();
	UpdateFinisherTag(false);

	CurrentNodeId = INDEX_NONE;
	ComboCounter = 0;
	WindowTimer = 0.0f;
	bHasBufferedInput = false;

	OnComboReset.Broadcast();
}

bool USBComboComponent::GetCurrentComboNode(FSBComboNode& OutNode) const
{
	if (CurrentNodeId == INDEX_NONE) return false;

	const FSBComboNode* Found = ActiveTree.Nodes.FindByPredicate([this](const FSBComboNode& Node)
	{
		return Node.NodeId == CurrentNodeId;
	});

	if (Found)
	{
		OutNode = *Found;
		return true;
	}
	return false;
}

float USBComboComponent::GetCurrentDamageMultiplier() const
{
	FSBComboNode CurrentNode;
	if (GetCurrentComboNode(CurrentNode))
	{
		return CurrentNode.DamageMultiplier;
	}
	return 1.0f;
}

void USBComboComponent::UpdateFinisherTag(bool bIsFinisher)
{
	if (!CachedStateComp.IsValid()) return;

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (bIsFinisher)
	{
		CachedStateComp->AddTag(Tags.State_Combat_FinisherReady);
	}
	else
	{
		CachedStateComp->RemoveTag(Tags.State_Combat_FinisherReady);
	}
}
