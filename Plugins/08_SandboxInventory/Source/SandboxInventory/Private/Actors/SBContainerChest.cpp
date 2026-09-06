#include "Actors/SBContainerChest.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

ASBContainerChest::ASBContainerChest()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetReplicates(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(100.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InventoryComponent = CreateDefaultSubobject<USBInventoryComponent>(TEXT("InventoryComponent"));

	MaxInteractionDistance = 300.0f;
}

void ASBContainerChest::BeginPlay()
{
	Super::BeginPlay();
}

void ASBContainerChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		return;
	}

	// Verifica se os jogadores que estão interagindo se afastaram demais
	for (int32 i = ActiveInteractors.Num() - 1; i >= 0; --i)
	{
		AActor* Interactor = ActiveInteractors[i].Get();
		if (!Interactor)
		{
			RemoveInteractorAt(i);
			continue;
		}

		float Distance = FVector::Dist(GetActorLocation(), Interactor->GetActorLocation());
		if (Distance > MaxInteractionDistance)
		{
			StopInteracting(Interactor);
		}
	}
}

bool ASBContainerChest::CanInteract_Implementation(AActor* Interactor) const
{
	if (!Interactor)
	{
		return false;
	}

	USBInventoryComponent* InvComp = Interactor->FindComponentByClass<USBInventoryComponent>();
	return InvComp != nullptr;
}

FText ASBContainerChest::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return FText::FromString(TEXT("Abrir Baú"));
}

void ASBContainerChest::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || !Interactor)
	{
		return;
	}

	USBStateComponent* StateComp = Interactor->FindComponentByClass<USBStateComponent>();
	if (StateComp)
	{
		StateComp->AddTag(FSBGameplayTags::Get().State_Character_Interacting);
	}

	if (!ActiveInteractors.Contains(Interactor))
	{
		ActiveInteractors.Add(Interactor);
	}
}

void ASBContainerChest::StopInteracting(AActor* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	if (HasAuthority())
	{
		USBStateComponent* StateComp = Interactor->FindComponentByClass<USBStateComponent>();
		if (StateComp)
		{
			StateComp->RemoveTag(FSBGameplayTags::Get().State_Character_Interacting);
		}

		ActiveInteractors.Remove(Interactor);
	}
}

void ASBContainerChest::RemoveInteractorAt(int32 Index)
{
	if (ActiveInteractors.IsValidIndex(Index))
	{
		ActiveInteractors.RemoveAt(Index);
	}
}
