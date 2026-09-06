#include "Actors/SBPhysicalLootDrop.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SBInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Items/SBItemFragment_Rarity.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SBGameplayTags.h"

ASBPhysicalLootDrop::ASBPhysicalLootDrop()
{
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(30.0f);
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(40.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetEnableGravity(true);
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StackCount = 1;
	InteractionDuration = 0.0f;
	bIsLocked = false;
}

void ASBPhysicalLootDrop::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASBPhysicalLootDrop, ItemDefinition);
	DOREPLIFETIME(ASBPhysicalLootDrop, StackCount);
}

void ASBPhysicalLootDrop::InitializeLoot(USBItemDefinition* InItemDef, int32 InStackCount)
{
	if (!HasAuthority() && !GIsAutomationTesting) return;

	ItemDefinition = InItemDef;
	StackCount = FMath::Max(1, InStackCount);
	bIsLocked = false;

	UpdateVisuals();
}

bool ASBPhysicalLootDrop::CanInteract_Implementation(AActor* Interactor) const
{
	if (bIsLocked || !ItemDefinition || StackCount <= 0 || !Interactor)
	{
		return false;
	}

	USBInventoryComponent* InventoryComp = Interactor->FindComponentByClass<USBInventoryComponent>();
	return InventoryComp != nullptr;
}

FText ASBPhysicalLootDrop::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	if (!ItemDefinition)
	{
		return FText::GetEmpty();
	}

	FText ItemName = ItemDefinition->DisplayName.IsEmpty() ? FText::FromString(ItemDefinition->GetName()) : ItemDefinition->DisplayName;
	return FText::FromString(FString::Printf(TEXT("Coletar %s (x%d)"), *ItemName.ToString(), StackCount));
}

void ASBPhysicalLootDrop::Interact_Implementation(AActor* Interactor)
{
	if ((!HasAuthority() && !GIsAutomationTesting) || bIsLocked || !ItemDefinition || StackCount <= 0 || !Interactor)
	{
		return;
	}

	USBInventoryComponent* InventoryComp = Interactor->FindComponentByClass<USBInventoryComponent>();
	if (InventoryComp)
	{
		// Ativa o lock imediatamente para evitar coletas concorrentes
		bIsLocked = true;

		USBItemInstance* AddedItem = InventoryComp->ServerAddItem(ItemDefinition, StackCount);
		if (AddedItem)
		{
			Destroy();
		}
		else
		{
			// Destrava se a adição falhou (ex: inventário cheio)
			bIsLocked = false;
		}
	}
}

void ASBPhysicalLootDrop::OnRep_ItemDefinition()
{
	UpdateVisuals();
}

FGameplayTag ASBPhysicalLootDrop::GetRarityTag() const
{
	if (ItemDefinition)
	{
		const USBItemFragment* FoundFrag = ItemDefinition->FindFragmentByClass(USBItemFragment_Rarity::StaticClass());
		if (const USBItemFragment_Rarity* RarityFrag = Cast<USBItemFragment_Rarity>(FoundFrag))
		{
			if (RarityFrag->RarityTag.IsValid())
			{
				return RarityFrag->RarityTag;
			}
		}
	}
	return FSBGameplayTags::Get().Loot_Rarity_Common;
}

FLinearColor ASBPhysicalLootDrop::GetRarityColor() const
{
	FGameplayTag RarityTag = GetRarityTag();
	const FSBGameplayTags& GameplayTags = FSBGameplayTags::Get();

	if (RarityTag == GameplayTags.Loot_Rarity_Uncommon)
	{
		return FLinearColor(0.1f, 0.8f, 0.1f); // Verde
	}
	else if (RarityTag == GameplayTags.Loot_Rarity_Rare)
	{
		return FLinearColor(0.1f, 0.4f, 0.9f); // Azul
	}
	else if (RarityTag == GameplayTags.Loot_Rarity_Epic)
	{
		return FLinearColor(0.6f, 0.1f, 0.8f); // Roxo
	}
	else if (RarityTag == GameplayTags.Loot_Rarity_Legendary)
	{
		return FLinearColor(1.0f, 0.5f, 0.0f); // Laranja/Ouro
	}

	return FLinearColor(0.7f, 0.7f, 0.7f); // Comum (Branco/Cinza)
}

void ASBPhysicalLootDrop::UpdateVisuals()
{
	if (MeshComponent)
	{
		UMaterialInterface* CurrentMaterial = MeshComponent->GetMaterial(0);
		if (CurrentMaterial)
		{
			UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0, CurrentMaterial);
			if (DynamicMaterial)
			{
				DynamicMaterial->SetVectorParameterValue(TEXT("RarityColor"), GetRarityColor());
			}
		}
	}
}
