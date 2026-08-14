#include "Components/SBAnimLayerManagerComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBMovementComponent.h"
#include "Animation/DataAssets/SBAnimLayerConfigDataAsset.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"

USBAnimLayerManagerComponent::USBAnimLayerManagerComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics; // Processa vinculações antes da simulação física
}

void USBAnimLayerManagerComponent::OnInitialize_Implementation()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		CachedStateComponent = Owner->FindComponentByClass<USBStateComponent>();

		// Prerrequisito de tick: Garante que o USBMovementComponent processe as mutações da pilha física
		// antes do AnimLayerManager atualizar as layers no mesmo tick
		UActorComponent* MovementComp = Owner->FindComponentByClass<USBMovementComponent>();
		if (MovementComp)
		{
			AddTickPrerequisiteComponent(MovementComp);
		}
	}

	// Inscreve-se nos callbacks de alteração de tags de estado
	if (CachedStateComponent)
	{
		CachedStateComponent->OnStateChanged.AddDynamic(this, &USBAnimLayerManagerComponent::OnStateTagChanged);
	}
}

void USBAnimLayerManagerComponent::OnReady_Implementation()
{
	// Garante uma reconstrução inicial quando todos os componentes estiverem prontos
	bRebuildPending = true;
}

void USBAnimLayerManagerComponent::OnShutdown_Implementation()
{
	if (CachedStateComponent)
	{
		CachedStateComponent->OnStateChanged.RemoveAll(this);
	}
}

void USBAnimLayerManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Coalesce: se houve alteração de tags ou links pendentes de frames anteriores, executa uma única vez
	if (bRebuildPending || bPendingInitialLink)
	{
		RebuildLinkedLayers();
	}
}

void USBAnimLayerManagerComponent::OnStateTagChanged(FGameplayTag Tag, bool bAdded)
{
	// Apenas sinaliza a necessidade de rebuild. Evita múltiplos updates no mesmo frame (tick)
	bRebuildPending = true;
}

UAnimInstance* USBAnimLayerManagerComponent::GetActiveAnimInstance() const
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		ACharacter* Character = Cast<ACharacter>(Owner);
		if (Character && Character->GetMesh())
		{
			return Character->GetMesh()->GetAnimInstance();
		}
	}
	return nullptr;
}

void USBAnimLayerManagerComponent::RebuildLinkedLayers()
{
	UAnimInstance* AnimInstance = GetActiveAnimInstance();
	if (!AnimInstance)
	{
		// Se a AnimInstance ainda não estiver pronta (inicialização assíncrona), agenda para o próximo frame
		bPendingInitialLink = true;
		return;
	}

	bPendingInitialLink = false;
	bRebuildPending = false;

	if (!AnimLayerConfigAsset || !CachedStateComponent)
	{
		return;
	}

	// 1. Estrutura temporária para armazenar layers que devem estar ativas
	struct FSBActiveLayerEntry
	{
		TSubclassOf<UAnimInstance> LayerClass;
		int32 Priority;
	};

	TArray<FSBActiveLayerEntry> ActiveLayers;

	// 2. Coleta layers cujas tags de estado correspondentes estão ativas no StateComponent
	for (const FSBAnimLayerMappingEntry& Entry : AnimLayerConfigAsset->LayerMappings)
	{
		if (Entry.AnimLayerClass && CachedStateComponent->HasTag(Entry.StateTag))
		{
			FSBActiveLayerEntry ActiveEntry;
			ActiveEntry.LayerClass = Entry.AnimLayerClass;
			ActiveEntry.Priority = Entry.Priority;
			ActiveLayers.Add(ActiveEntry);
		}
	}

	// 3. Se nenhuma layer está ativa, desvincula tudo o que estava carregado
	if (ActiveLayers.Num() == 0)
	{
		if (CurrentLinkedClasses.Num() > 0)
		{
			for (TSubclassOf<UAnimInstance> LinkedClass : CurrentLinkedClasses)
			{
				AnimInstance->UnlinkAnimClassLayers(LinkedClass);
			}
			CurrentLinkedClasses.Empty();
		}
		return;
	}

	// 4. Executa StableSort determinístico baseado em prioridade ascendente
	// (Layers de menor prioridade ligam primeiro, prioridades maiores por último vencendo conflitos de funções)
	ActiveLayers.StableSort([](const FSBActiveLayerEntry& A, const FSBActiveLayerEntry& B)
	{
		return A.Priority < B.Priority;
	});

	// 5. Extrai as classes ordenadas unicamente
	TArray<TSubclassOf<UAnimInstance>> TargetClasses;
	for (const FSBActiveLayerEntry& Entry : ActiveLayers)
	{
		TargetClasses.AddUnique(Entry.LayerClass);
	}

	// 6. Hitch-Free Check: Se o conjunto ordenado de layers não mudou, evita desvincular/re-vincular
	if (CurrentLinkedClasses == TargetClasses)
	{
		return;
	}

	// 7. Executa desvinculações das layers anteriores
	for (TSubclassOf<UAnimInstance> LinkedClass : CurrentLinkedClasses)
	{
		AnimInstance->UnlinkAnimClassLayers(LinkedClass);
	}

	// 8. Vincula novas layers na ordem correta de prioridades
	for (TSubclassOf<UAnimInstance> TargetClass : TargetClasses)
	{
		if (TargetClass && TargetClass->ImplementsInterface(USBAnimLayerInterface::StaticClass()))
		{
			AnimInstance->LinkAnimClassLayers(TargetClass);
		}
	}

	// Salva o estado atualizado
	CurrentLinkedClasses = TargetClasses;
}
