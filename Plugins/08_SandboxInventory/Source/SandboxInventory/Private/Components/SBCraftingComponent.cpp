#include "Components/SBCraftingComponent.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBStateComponent.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemFragment_Salvageable.h"
#include "Items/SBItemFragment_Durability.h"
#include "Items/SBItemFragment_Upgrade.h"
#include "Components/SBAttributeComponent.h"

USBCraftingComponent::USBCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USBCraftingComponent::OnInitialize_Implementation()
{
	InitializeComponent_Implementation();
}

void USBCraftingComponent::OnReady_Implementation()
{
	if (!CachedInventoryComponent && GetOwner())
	{
		CachedInventoryComponent = GetOwner()->FindComponentByClass<USBInventoryComponent>();
	}

	if (!CachedStateComponent && GetOwner())
	{
		CachedStateComponent = GetOwner()->FindComponentByClass<USBStateComponent>();
	}
}

void USBCraftingComponent::OnShutdown_Implementation()
{
	CachedInventoryComponent = nullptr;
	CachedStateComponent = nullptr;
}

void USBCraftingComponent::InitializeComponent_Implementation()
{
	if (GetOwner())
	{
		CachedInventoryComponent = GetOwner()->FindComponentByClass<USBInventoryComponent>();
		CachedStateComponent = GetOwner()->FindComponentByClass<USBStateComponent>();
	}
}

void USBCraftingComponent::ResetComponent_Implementation()
{
	CachedInventoryComponent = nullptr;
	CachedStateComponent = nullptr;
}

USBEventSubsystem* USBCraftingComponent::GetEventSubsystem() const
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<USBEventSubsystem>();
		}
	}
	return nullptr;
}

bool USBCraftingComponent::CanCraftRecipe(const USBCraftingRecipeDataAsset* Recipe) const
{
	if (!Recipe || !Recipe->ResultItemDef || Recipe->ResultQuantity <= 0)
	{
		return false;
	}

	USBInventoryComponent* InvComp = CachedInventoryComponent;
	if (!InvComp && GetOwner())
	{
		InvComp = GetOwner()->FindComponentByClass<USBInventoryComponent>();
	}

	if (!InvComp)
	{
		return false;
	}

	// Validação de Estação de Trabalho
	if (Recipe->RequiredStationTag.IsValid())
	{
		USBStateComponent* StateComp = CachedStateComponent;
		if (!StateComp && GetOwner())
		{
			StateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
		}

		if (!StateComp || !StateComp->HasTag(Recipe->RequiredStationTag))
		{
			return false;
		}
	}

	// Validação de Ingredientes
	for (const FSBCraftingIngredient& Ingredient : Recipe->Ingredients)
	{
		if (!Ingredient.ItemDef || Ingredient.Quantity <= 0)
		{
			return false;
		}

		if (InvComp->GetTotalItemQuantity(Ingredient.ItemDef) < Ingredient.Quantity)
		{
			return false;
		}
	}

	return true;
}

bool USBCraftingComponent::ServerCraftRecipe(const USBCraftingRecipeDataAsset* Recipe)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	if (!CanCraftRecipe(Recipe))
	{
		OnCraftingFailed.Broadcast(Recipe, TEXT("Pré-requisitos de estação ou ingredientes insuficientes"));

		if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
		{
			USBCraftingEventPayload* FailPayload = NewObject<USBCraftingEventPayload>(this);
			FailPayload->TargetPawn = Cast<APawn>(Owner);
			FailPayload->RecipeTag = Recipe ? Recipe->RecipeTag : FGameplayTag::EmptyTag;
			FailPayload->ResultItemDef = Recipe ? Recipe->ResultItemDef : nullptr;
			FailPayload->ResultQuantity = 0;
			FailPayload->bSuccess = false;

			EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Crafting_Failed, FailPayload);
		}

		return false;
	}

	USBInventoryComponent* InvComp = CachedInventoryComponent;
	if (!InvComp)
	{
		InvComp = Owner->FindComponentByClass<USBInventoryComponent>();
	}

	if (!InvComp)
	{
		return false;
	}

	// 1. Consumo seguro e atômico dos materiais
	for (const FSBCraftingIngredient& Ingredient : Recipe->Ingredients)
	{
		InvComp->ServerConsumeItemQuantity(Ingredient.ItemDef, Ingredient.Quantity);
	}

	// 2. Criação e concessão do item resultante
	USBItemInstance* CreatedInstance = InvComp->ServerAddItem(Recipe->ResultItemDef, Recipe->ResultQuantity);

	// 3. Notificação via Delegates
	OnCraftingCompleted.Broadcast(Recipe, CreatedInstance, Recipe->ResultQuantity);

	// 4. Notificação via Event Bus com zero-allocation
	if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
	{
		USBCraftingEventPayload* SuccessPayload = NewObject<USBCraftingEventPayload>(this);
		SuccessPayload->TargetPawn = Cast<APawn>(Owner);
		SuccessPayload->RecipeTag = Recipe->RecipeTag;
		SuccessPayload->ResultItemDef = Recipe->ResultItemDef;
		SuccessPayload->ResultQuantity = Recipe->ResultQuantity;
		SuccessPayload->bSuccess = true;

		EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Crafting_Completed, SuccessPayload);
	}

	return true;
}

bool USBCraftingComponent::ServerSalvageItem(USBItemInstance* ItemInstance, int32 Quantity)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	if (!ItemInstance || Quantity <= 0 || ItemInstance->StackCount < Quantity)
	{
		OnSalvagingFailed.Broadcast(nullptr, TEXT("ItemInstance nulo ou quantidade inválida"));
		return false;
	}

	const USBItemDefinition* ItemDef = ItemInstance->ItemDef;
	if (!ItemDef)
	{
		OnSalvagingFailed.Broadcast(nullptr, TEXT("Definição de item nula"));
		return false;
	}

	const USBItemFragment_Salvageable* SalvageFragment = Cast<USBItemFragment_Salvageable>(ItemDef->FindFragmentByClass(USBItemFragment_Salvageable::StaticClass()));
	if (!SalvageFragment)
	{
		OnSalvagingFailed.Broadcast(ItemDef, TEXT("Item não é desmantelável"));
		return false;
	}

	USBInventoryComponent* InvComp = CachedInventoryComponent;
	if (!InvComp)
	{
		InvComp = Owner->FindComponentByClass<USBInventoryComponent>();
	}

	if (!InvComp)
	{
		OnSalvagingFailed.Broadcast(ItemDef, TEXT("Inventário indisponível"));
		return false;
	}

	// 1. Remove a quantidade exata do inventário
	if (!InvComp->ServerRemoveItem(ItemInstance, Quantity))
	{
		OnSalvagingFailed.Broadcast(ItemDef, TEXT("Erro ao remover item do inventário"));
		return false;
	}

	TArray<USBItemInstance*> GainedItems;

	// 2. Roda a probabilidade para cada unidade desmantelada
	for (int32 i = 0; i < Quantity; ++i)
	{
		for (const FSBSalvageOutcome& Outcome : SalvageFragment->PotentialOutcomes)
		{
			if (!Outcome.ItemDef)
			{
				continue;
			}

			float Roll = FMath::FRand();
			if (Roll <= Outcome.Probability)
			{
				int32 GainedQty = FMath::RandRange(Outcome.MinQuantity, Outcome.MaxQuantity);
				if (GainedQty > 0)
				{
					USBItemInstance* GainedInstance = InvComp->ServerAddItem(Outcome.ItemDef, GainedQty);
					if (GainedInstance)
					{
						GainedItems.Add(GainedInstance);
					}
				}
			}
		}
	}

	// 3. Notifica via Delegate síncrono C++
	OnSalvagingCompleted.Broadcast(ItemDef, Quantity, GainedItems);

	return true;
}

bool USBCraftingComponent::ServerRepairItem(USBItemInstance* ItemInstance)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	if (!ItemInstance)
	{
		return false;
	}

	// 1. Valida se o item suporta durabilidade
	const USBItemFragment_Durability* DurabilityFragment = Cast<USBItemFragment_Durability>(ItemInstance->FindFragmentByClass(USBItemFragment_Durability::StaticClass()));
	if (!DurabilityFragment)
	{
		return false;
	}

	// 2. Valida proximidade da bancada de trabalho (presença de alguma tag de estação)
	USBStateComponent* StateComp = CachedStateComponent;
	if (!StateComp)
	{
		StateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	bool bNearStation = false;
	if (StateComp)
	{
		// Se o personagem tem qualquer tag correspondente a "Crafting.Station" ou "State.Station"
		const FGameplayTagContainer& ActorTags = StateComp->GetActiveStateTags();
		for (const FGameplayTag& Tag : ActorTags)
		{
			FString TagName = Tag.ToString();
			if (TagName.StartsWith(TEXT("Crafting.Station")) || TagName.StartsWith(TEXT("State.Station")))
			{
				bNearStation = true;
				break;
			}
		}
	}

	if (!bNearStation)
	{
		return false;
	}

	// 3. Executa o reparo
	ItemInstance->SetDurability_Implementation(DurabilityFragment->MaxDurability);

	// 4. Notifica o inventário para replicação
	USBInventoryComponent* InvComp = CachedInventoryComponent;
	if (!InvComp)
	{
		InvComp = Owner->FindComponentByClass<USBInventoryComponent>();
	}
	if (InvComp)
	{
		InvComp->MarkItemInstanceUpdated(ItemInstance);

		// Se o item reparado for armadura e estiver equipado, re-aplica modificadores
		if (ItemInstance->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped))
		{
			InvComp->ServerEquipItem(ItemInstance);
		}
	}

	return true;
}

bool USBCraftingComponent::ServerUpgradeItem(USBItemInstance* ItemInstance)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	if (!ItemInstance || !ItemInstance->ItemDef)
	{
		return false;
	}

	// 1. Valida se o item suporta upgrade
	const USBItemFragment_Upgrade* UpgradeFrag = Cast<USBItemFragment_Upgrade>(ItemInstance->FindFragmentByClass(USBItemFragment_Upgrade::StaticClass()));
	if (!UpgradeFrag)
	{
		return false;
	}

	// 2. Valida se ainda há níveis disponíveis para upgrade
	int32 NextLevelIndex = ItemInstance->UpgradeLevel;
	if (NextLevelIndex < 0 || NextLevelIndex >= UpgradeFrag->UpgradeLevels.Num())
	{
		return false;
	}

	const FSBUpgradeCostPerLevel& UpgradeCost = UpgradeFrag->UpgradeLevels[NextLevelIndex];

	// 3. Valida proximidade da bancada de trabalho exigida (Station Tag)
	if (UpgradeFrag->RequiredStationTag.IsValid())
	{
		USBStateComponent* StateComp = CachedStateComponent;
		if (!StateComp)
		{
			StateComp = Owner->FindComponentByClass<USBStateComponent>();
		}
		if (!StateComp || !StateComp->GetActiveStateTags().HasTag(UpgradeFrag->RequiredStationTag))
		{
			return false;
		}
	}

	// 4. Valida componentes de inventário e atributos do jogador
	USBInventoryComponent* InvComp = CachedInventoryComponent;
	if (!InvComp)
	{
		InvComp = Owner->FindComponentByClass<USBInventoryComponent>();
	}
	if (!InvComp)
	{
		return false;
	}

	USBAttributeComponent* AttrComp = Owner->FindComponentByClass<USBAttributeComponent>();
	if (UpgradeCost.CoinCost > 0)
	{
		if (!AttrComp)
		{
			return false;
		}
		FGameplayTag CoinsTag = FSBGameplayTags::Get().Attribute_Coins;
		if (AttrComp->GetAttributeValue(CoinsTag) < UpgradeCost.CoinCost)
		{
			return false;
		}
	}

	for (const FSBCraftingIngredient& Ingredient : UpgradeCost.Ingredients)
	{
		if (!Ingredient.ItemDef || InvComp->GetTotalItemQuantity(Ingredient.ItemDef) < Ingredient.Quantity)
		{
			return false;
		}
	}

	// 5. Consome os insumos e moedas
	for (const FSBCraftingIngredient& Ingredient : UpgradeCost.Ingredients)
	{
		InvComp->ServerConsumeItemQuantity(Ingredient.ItemDef, Ingredient.Quantity);
	}

	if (UpgradeCost.CoinCost > 0 && AttrComp)
	{
		FGameplayTag CoinsTag = FSBGameplayTags::Get().Attribute_Coins;
		FSBAttribute CoinsAttr;
		if (AttrComp->GetAttribute(CoinsTag, CoinsAttr))
		{
			float NewCoins = CoinsAttr.BaseValue - UpgradeCost.CoinCost;
			AttrComp->SetAttributeBaseValue(CoinsTag, NewCoins);
		}
	}

	// 6. Verifica se o item está equipado
	FGameplayTag EquippedTag = FSBGameplayTags::Get().State_Item_Equipped;
	bool bIsEquipped = EquippedTag.IsValid() && ItemInstance->DynamicTags.HasTagExact(EquippedTag);

	if (bIsEquipped)
	{
		InvComp->ServerUnequipItem(ItemInstance);
	}

	// 7. Incrementa o nível de upgrade e restaura durabilidade se aplicável
	ItemInstance->UpgradeLevel++;

	const USBItemFragment_Durability* DurabilityFragment = Cast<USBItemFragment_Durability>(ItemInstance->FindFragmentByClass(USBItemFragment_Durability::StaticClass()));
	if (DurabilityFragment)
	{
		ItemInstance->SetDurability_Implementation(DurabilityFragment->MaxDurability);
	}

	// 8. Re-equipa se estava equipado para re-aplicar modificadores escalados
	if (bIsEquipped)
	{
		InvComp->ServerEquipItem(ItemInstance);
	}

	// 9. Atualiza o inventário para replicar
	InvComp->MarkItemInstanceUpdated(ItemInstance);

	return true;
}
