#include "Components/SBStructuralIntegrityComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBStructuralIntegrityComponent::USBStructuralIntegrityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBStructuralIntegrityComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	StructuralData.MaxLoadCapacity = Settings.BaseLoadCapacity;
	StructuralData.MaxSupportDistance = Settings.MaxHorizontalSpan;

	TArray<USBStructuralIntegrityComponent*> Visited;
	RecalculateIntegrity(Visited);
}

void USBStructuralIntegrityComponent::OnShutdown_Implementation()
{
	TriggerStructuralCollapse();
}

void USBStructuralIntegrityComponent::SetGroundAnchor(bool bAnchor)
{
	StructuralData.bIsGroundAnchor = bAnchor;
	if (bAnchor)
	{
		StructuralData.DistanceFromAnchor = 0;
		StructuralData.StructuralStability = 100.0f;
		StructuralData.StabilityState = ESBStructuralStabilityState::Stable;
	}

	TArray<USBStructuralIntegrityComponent*> Visited;
	RecalculateIntegrity(Visited);
}

void USBStructuralIntegrityComponent::RegisterNeighborPiece(USBStructuralIntegrityComponent* NeighborComp)
{
	if (!NeighborComp || NeighborComp == this || ConnectedNeighbors.Contains(NeighborComp))
	{
		return;
	}

	ConnectedNeighbors.Add(NeighborComp);
	if (!NeighborComp->ConnectedNeighbors.Contains(this))
	{
		NeighborComp->ConnectedNeighbors.Add(this);
	}

	TArray<USBStructuralIntegrityComponent*> Visited;
	RecalculateIntegrity(Visited);
}

void USBStructuralIntegrityComponent::UnregisterNeighborPiece(USBStructuralIntegrityComponent* NeighborComp)
{
	if (!NeighborComp)
	{
		return;
	}

	ConnectedNeighbors.Remove(NeighborComp);
	NeighborComp->ConnectedNeighbors.Remove(this);

	TArray<USBStructuralIntegrityComponent*> Visited;
	RecalculateIntegrity(Visited);
}

void USBStructuralIntegrityComponent::AddSupportedLoad(float LoadMass)
{
	StructuralData.CurrentLoadWeight += LoadMass;
	TArray<USBStructuralIntegrityComponent*> Visited;
	RecalculateIntegrity(Visited);
	OnStructuralLoadChanged.Broadcast(StructuralData.CurrentLoadWeight, StructuralData.MaxLoadCapacity);
}

void USBStructuralIntegrityComponent::RemoveSupportedLoad(float LoadMass)
{
	StructuralData.CurrentLoadWeight = FMath::Max(0.0f, StructuralData.CurrentLoadWeight - LoadMass);
	TArray<USBStructuralIntegrityComponent*> Visited;
	RecalculateIntegrity(Visited);
	OnStructuralLoadChanged.Broadcast(StructuralData.CurrentLoadWeight, StructuralData.MaxLoadCapacity);
}

void USBStructuralIntegrityComponent::GatherStructuralNetwork(TArray<USBStructuralIntegrityComponent*>& OutNetwork)
{
	if (OutNetwork.Contains(this))
	{
		return;
	}

	OutNetwork.Add(this);
	for (const TWeakObjectPtr<USBStructuralIntegrityComponent>& NeighborPtr : ConnectedNeighbors)
	{
		if (NeighborPtr.IsValid())
		{
			NeighborPtr->GatherStructuralNetwork(OutNetwork);
		}
	}
}

void USBStructuralIntegrityComponent::RecalculateIntegrity(TArray<USBStructuralIntegrityComponent*>& Visited)
{
	// A versao anterior propagava lendo a DistanceFromAnchor ATUAL dos vizinhos. Como as
	// ligacoes sao bidirecionais, isso criava suporte circular: ao remover a ancora, a
	// fundacao lia a distancia antiga da parede -- que viera da propria fundacao -- e a
	// estrutura continuava "apoiada" em si mesma. Destruir a base nao derrubava nada.
	//
	// Agora sao tres fases: coletar a rede, invalidar tudo que nao e ancora, e so entao
	// propagar em largura a partir das ancoras. Nenhum valor obsoleto sobrevive.

	// Fase 1 -- coleta a rede conectada.
	GatherStructuralNetwork(Visited);

	// Fase 2 -- invalida distancias; apenas ancoras nascem validas.
	for (USBStructuralIntegrityComponent* Piece : Visited)
	{
		if (!Piece)
		{
			continue;
		}

		if (Piece->StructuralData.bIsGroundAnchor)
		{
			Piece->StructuralData.DistanceFromAnchor = 0;
			Piece->StructuralData.StructuralStability = 100.0f;
		}
		else
		{
			Piece->StructuralData.DistanceFromAnchor = INDEX_NONE;
			Piece->StructuralData.StructuralStability = 0.0f;
		}
	}

	// Fase 3 -- propaga em largura a partir das ancoras.
	TArray<USBStructuralIntegrityComponent*> Fila;
	for (USBStructuralIntegrityComponent* Piece : Visited)
	{
		if (Piece && Piece->StructuralData.bIsGroundAnchor)
		{
			Fila.Add(Piece);
		}
	}

	for (int32 Index = 0; Index < Fila.Num(); ++Index)
	{
		USBStructuralIntegrityComponent* Atual = Fila[Index];
		if (!Atual)
		{
			continue;
		}

		const int32 DistAtual = Atual->StructuralData.DistanceFromAnchor;
		for (const TWeakObjectPtr<USBStructuralIntegrityComponent>& NeighborPtr : Atual->ConnectedNeighbors)
		{
			USBStructuralIntegrityComponent* Vizinho = NeighborPtr.Get();
			if (!Vizinho || Vizinho->StructuralData.bIsGroundAnchor)
			{
				continue;
			}

			// Mesma regra de alcance da versao anterior: o suporte so alcanca o vizinho se a
			// distancia de quem apoia for menor que o alcance maximo dele.
			if (DistAtual >= Vizinho->StructuralData.MaxSupportDistance)
			{
				continue;
			}

			const int32 NovaDist = DistAtual + 1;
			if (Vizinho->StructuralData.DistanceFromAnchor == INDEX_NONE || NovaDist < Vizinho->StructuralData.DistanceFromAnchor)
			{
				Vizinho->StructuralData.DistanceFromAnchor = NovaDist;
				const float DistanceDecay = 1.0f - (float)NovaDist / (float)(Vizinho->StructuralData.MaxSupportDistance + 1);
				Vizinho->StructuralData.StructuralStability = FMath::Clamp(DistanceDecay * 100.0f, 0.0f, 100.0f);
				Fila.Add(Vizinho);
			}
		}
	}

	// Fase 4 -- aplica estado e tags. Copia a lista: o colapso mexe em ConnectedNeighbors.
	TArray<USBStructuralIntegrityComponent*> Instantaneo = Visited;
	for (USBStructuralIntegrityComponent* Piece : Instantaneo)
	{
		if (Piece)
		{
			Piece->ApplyStabilityState();
		}
	}
}

void USBStructuralIntegrityComponent::ApplyStabilityState()
{
	if (StructuralData.DistanceFromAnchor == INDEX_NONE || StructuralData.StructuralStability <= 0.0f)
	{
		TriggerStructuralCollapse();
		return;
	}

	const float LoadRatio = StructuralData.MaxLoadCapacity > 0.0f ? (StructuralData.CurrentLoadWeight / StructuralData.MaxLoadCapacity) : 0.0f;
	const ESBStructuralStabilityState PrevState = StructuralData.StabilityState;

	if (LoadRatio >= Settings.CriticalThreshold)
	{
		StructuralData.StabilityState = ESBStructuralStabilityState::Critical;
	}
	else if (LoadRatio >= Settings.StressThreshold)
	{
		StructuralData.StabilityState = ESBStructuralStabilityState::Stressed;
	}
	else
	{
		StructuralData.StabilityState = ESBStructuralStabilityState::Stable;
	}

	if (PrevState != StructuralData.StabilityState)
	{
		OnStructuralStabilityChanged.Broadcast(StructuralData.StabilityState);
	}

	SyncStructuralTags();
}

void USBStructuralIntegrityComponent::TriggerStructuralCollapse()
{
	if (StructuralData.StabilityState == ESBStructuralStabilityState::Collapsing)
	{
		return;
	}

	StructuralData.StabilityState = ESBStructuralStabilityState::Collapsing;
	StructuralData.StructuralStability = 0.0f;
	StructuralData.DistanceFromAnchor = INDEX_NONE;

	SyncStructuralTags();
	OnStructuralCollapse.Broadcast();
	OnStructuralStabilityChanged.Broadcast(StructuralData.StabilityState);

	TArray<TWeakObjectPtr<USBStructuralIntegrityComponent>> NeighborsCopy = ConnectedNeighbors;
	ConnectedNeighbors.Empty();

	for (const TWeakObjectPtr<USBStructuralIntegrityComponent>& NeighborPtr : NeighborsCopy)
	{
		if (NeighborPtr.IsValid())
		{
			NeighborPtr->ConnectedNeighbors.Remove(this);
			TArray<USBStructuralIntegrityComponent*> Visited;
			NeighborPtr->RecalculateIntegrity(Visited);
		}
	}
}

void USBStructuralIntegrityComponent::SyncStructuralTags()
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

	CachedStateComp->RemoveTag(Tags.State_Building_Anchor);
	CachedStateComp->RemoveTag(Tags.State_Building_Supported);
	CachedStateComp->RemoveTag(Tags.State_Building_Stressed);
	CachedStateComp->RemoveTag(Tags.State_Building_Collapsing);

	if (StructuralData.bIsGroundAnchor)
	{
		CachedStateComp->AddTag(Tags.State_Building_Anchor);
	}

	if (StructuralData.StabilityState == ESBStructuralStabilityState::Collapsing)
	{
		CachedStateComp->AddTag(Tags.State_Building_Collapsing);
	}
	else
	{
		if (IsSupported())
		{
			CachedStateComp->AddTag(Tags.State_Building_Supported);
		}

		if (StructuralData.StabilityState == ESBStructuralStabilityState::Stressed || StructuralData.StabilityState == ESBStructuralStabilityState::Critical)
		{
			CachedStateComp->AddTag(Tags.State_Building_Stressed);
		}
	}
}
