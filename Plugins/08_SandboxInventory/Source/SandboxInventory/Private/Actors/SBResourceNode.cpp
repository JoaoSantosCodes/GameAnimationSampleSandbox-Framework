// Copyright 2026 João Santos. All Rights Reserved.
#include "Actors/SBResourceNode.h"
#include "Interfaces/SBCombatComponentInterface.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DataAssets/SBLootTableDataAsset.h"
#include "Components/SBInventoryComponent.h"
#include "Actors/SBPhysicalLootDrop.h"
#include "Subsystems/SBSandboxBackgroundSimSubsystem.h"
#include "Components/SBPersistenceComponent.h"

ASBResourceNode::ASBResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	RootComponent = StaticMeshComp;
}

void ASBResourceNode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASBResourceNode, Health);
	DOREPLIFETIME(ASBResourceNode, bIsDepleted);
}

void ASBResourceNode::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	if (HasAuthority())
	{
		if (USBSandboxBackgroundSimSubsystem* SimSubsystem = GetWorld()->GetSubsystem<USBSandboxBackgroundSimSubsystem>())
		{
			if (USBPersistenceComponent* PersistComp = FindComponentByClass<USBPersistenceComponent>())
			{
				if (PersistComp->PersistentId.IsValid())
				{
					FSBSimulatedEntityData SimData;
					if (SimSubsystem->GetSimulatedEntityData(PersistComp->PersistentId.Guid, SimData))
					{
						ResumeFromBackgroundSim_Implementation(SimData);
					}
				}
			}
		}
	}
}

float ASBResourceNode::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDepleted || !HasAuthority())
	{
		return 0.0f;
	}

	float FinalDamage = DamageAmount;

	// Validação de ferramenta por contrato, sem dependência de compilação com 06_SandboxCombat
	if (RequiredToolTag.IsValid() && DamageCauser)
	{
		bool bCorrectToolEquipped = false;

		if (DebugActiveToolTag.IsValid())
		{
			bCorrectToolEquipped = DebugActiveToolTag.MatchesTagExact(RequiredToolTag);
		}
		else
		{
			// Resolucao por contrato: 08_SandboxInventory e 06_SandboxCombat sao Extensoes de
			// Gameplay irmas e nao podem se referenciar diretamente (Principio 4). A pergunta
			// e respondida dentro do proprio modulo de combate, que e onde a semantica de
			// arma pertence.
			if (UActorComponent* CombatComp = DamageCauser->FindComponentByInterface(USBCombatComponentInterface::StaticClass()))
			{
				bCorrectToolEquipped = ISBCombatComponentInterface::Execute_HasActiveWeaponWithTag(CombatComp, RequiredToolTag);
			}
		}

		if (!bCorrectToolEquipped)
		{
			// Mitiga o dano em 90% se a ferramenta estiver incorreta
			FinalDamage *= 0.1f;
		}
	}

	Health = FMath::Max(0.0f, Health - FinalDamage);

	// Multi-cast / OnRep_Health cuidará dos efeitos no cliente
	OnRep_Health(Health + FinalDamage);

	if (Health <= 0.0f)
	{
		HandleDepletion(DamageCauser);
	}

	return FinalDamage;
}

void ASBResourceNode::OnRep_Health(float OldHealth)
{
	// Aqui poderia tocar som de impacto ou vfx baseando-se no material físico.
}

void ASBResourceNode::HandleDepletion(AActor* DamageCauser)
{
	bIsDepleted = true;
	OnRep_IsDepleted();

	if (!HasAuthority()) return;

	// Concede o loot para o jogador que causou o dano esgotante
	if (LootTable && DamageCauser)
	{
		TArray<FSBLootDropResult> DroppedItems = LootTable->RollLoot(1);
		USBInventoryComponent* InvComp = DamageCauser->FindComponentByClass<USBInventoryComponent>();

		for (const FSBLootDropResult& Drop : DroppedItems)
		{
			if (Drop.ItemDefinition && Drop.StackCount > 0)
			{
				bool bAddedToInventory = false;
				if (InvComp)
				{
					USBItemInstance* AddedItem = InvComp->ServerAddItem(Drop.ItemDefinition, Drop.StackCount);
					if (AddedItem)
					{
						bAddedToInventory = true;
					}
				}

				// Spawn físico se o jogador não tiver inventário ou se a inserção falhar (ex: mochila cheia)
				if (!bAddedToInventory)
				{
					FVector SpawnLoc = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
					FRotator SpawnRot = FRotator::ZeroRotator;
					ASBPhysicalLootDrop* PhysicalDrop = GetWorld()->SpawnActor<ASBPhysicalLootDrop>(ASBPhysicalLootDrop::StaticClass(), SpawnLoc, SpawnRot);
					if (PhysicalDrop)
					{
						PhysicalDrop->InitializeLoot(Drop.ItemDefinition, Drop.StackCount);
					}
				}
			}
		}
	}

	// Programa o Respawn do Recurso
	if (RespawnTime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ASBResourceNode::HandleRespawn, RespawnTime, false);

		if (USBSandboxBackgroundSimSubsystem* SimSubsystem = GetWorld()->GetSubsystem<USBSandboxBackgroundSimSubsystem>())
		{
			if (USBPersistenceComponent* PersistComp = FindComponentByClass<USBPersistenceComponent>())
			{
				if (PersistComp->PersistentId.IsValid())
				{
					FSBSimulatedEntityData SimData;
					// Chamada do ator para si mesmo: invoca o override diretamente. O despacho por
					// reflexao (Execute_) nao alcancava o _Implementation desta classe e devolvia
					// a implementacao default da interface, deixando SimData vazio.
					PrepareForBackgroundSim_Implementation(SimData);
					SimSubsystem->RegisterSimulatedEntity(SimData);
				}
			}
		}
	}
}

void ASBResourceNode::HandleRespawn()
{
	bIsDepleted = false;
	Health = MaxHealth;
	OnRep_IsDepleted();

	if (USBSandboxBackgroundSimSubsystem* SimSubsystem = GetWorld()->GetSubsystem<USBSandboxBackgroundSimSubsystem>())
	{
		if (USBPersistenceComponent* PersistComp = FindComponentByClass<USBPersistenceComponent>())
		{
			if (PersistComp->PersistentId.IsValid())
			{
				SimSubsystem->UnregisterSimulatedEntity(PersistComp->PersistentId.Guid);
			}
		}
	}
}

void ASBResourceNode::OnRep_IsDepleted()
{
	if (bIsDepleted)
	{
		StaticMeshComp->SetVisibility(false);
		StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		StaticMeshComp->SetVisibility(true);
		StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void ASBResourceNode::PrepareForBackgroundSim_Implementation(FSBSimulatedEntityData& OutData)
{
	if (USBPersistenceComponent* PersistComp = FindComponentByClass<USBPersistenceComponent>())
	{
		OutData.EntityId = PersistComp->PersistentId.Guid;
	}
	OutData.EntitySimType = FGameplayTag::RequestGameplayTag("SimType.ResourceNode");
	
	float RemainingTime = 0.0f;
	if (GetWorldTimerManager().IsTimerActive(RespawnTimerHandle))
	{
		RemainingTime = GetWorldTimerManager().GetTimerRemaining(RespawnTimerHandle);
	}
	OutData.NumericStates.Add(FGameplayTag::RequestGameplayTag("State.Timer.Respawn"), RemainingTime);
}

void ASBResourceNode::ResumeFromBackgroundSim_Implementation(const FSBSimulatedEntityData& InData)
{
	const float* FoundTimer = InData.NumericStates.Find(FGameplayTag::RequestGameplayTag("State.Timer.Respawn"));
	float RemainingTime = FoundTimer ? *FoundTimer : 0.0f;

	if (RemainingTime <= 0.0f)
	{
		bIsDepleted = false;
		Health = MaxHealth;
		OnRep_IsDepleted();

		if (USBSandboxBackgroundSimSubsystem* SimSubsystem = GetWorld()->GetSubsystem<USBSandboxBackgroundSimSubsystem>())
		{
			SimSubsystem->UnregisterSimulatedEntity(InData.EntityId);
		}
	}
	else
	{
		bIsDepleted = true;
		Health = 0.0f;
		OnRep_IsDepleted();

		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ASBResourceNode::HandleRespawn, RemainingTime, false);
	}
}

void ASBResourceNode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && bIsDepleted)
	{
		if (USBSandboxBackgroundSimSubsystem* SimSubsystem = GetWorld()->GetSubsystem<USBSandboxBackgroundSimSubsystem>())
		{
			if (USBPersistenceComponent* PersistComp = FindComponentByClass<USBPersistenceComponent>())
			{
				if (PersistComp->PersistentId.IsValid())
				{
					FSBSimulatedEntityData SimData;
					// Chamada do ator para si mesmo: invoca o override diretamente. O despacho por
					// reflexao (Execute_) nao alcancava o _Implementation desta classe e devolvia
					// a implementacao default da interface, deixando SimData vazio.
					PrepareForBackgroundSim_Implementation(SimData);
					SimSubsystem->RegisterSimulatedEntity(SimData);
				}
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}
