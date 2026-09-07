// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBMerchantComponent.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemInstance.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBAttributeComponent.h"
#include "SBGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

USBMerchantComponent::USBMerchantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	BuyPriceMultiplier = 1.0f;
	SellPriceMultiplier = 0.5f;
}

bool USBMerchantComponent::ServerBuyItem_Validate(APawn* PlayerPawn, const USBItemDefinition* ItemDef, int32 Quantity)
{
	return true;
}

void USBMerchantComponent::ServerBuyItem_Implementation(APawn* PlayerPawn, const USBItemDefinition* ItemDef, int32 Quantity)
{
	if (!PlayerPawn || !ItemDef || Quantity <= 0) return;

	// 1. Validação de Proximidade Física
	float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation());
	if (Dist > 400.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("USBMerchantComponent::ServerBuyItem: Rejected - Player too far (%.1f)"), Dist);
		return;
	}

	// 2. Validação de Disponibilidade e Preço
	if (!AvailableItems.Contains(ItemDef))
	{
		UE_LOG(LogTemp, Warning, TEXT("USBMerchantComponent::ServerBuyItem: Rejected - Item not sold by merchant"));
		return;
	}

	int32 BasePrice = AvailableItems[ItemDef];
	int32 TotalPrice = FMath::RoundToInt(BasePrice * BuyPriceMultiplier) * Quantity;

	// 3. Validação de Saldo/Coins do Atributo Replicado
	USBAttributeComponent* AttrComp = PlayerPawn->FindComponentByClass<USBAttributeComponent>();
	if (!AttrComp) return;

	FGameplayTag CoinsTag = FSBGameplayTags::Get().Attribute_Coins;
	float PlayerCoins = AttrComp->GetAttributeValue(CoinsTag);

	if (PlayerCoins < TotalPrice)
	{
		UE_LOG(LogTemp, Warning, TEXT("USBMerchantComponent::ServerBuyItem: Rejected - Insufficient coins (%.1f / %d)"), PlayerCoins, TotalPrice);
		return;
	}

	// 4. Validação e Concessão no Inventário
	USBInventoryComponent* InvComp = PlayerPawn->FindComponentByClass<USBInventoryComponent>();
	if (!InvComp) return;

	// Debita Moedas de forma autoritativa no servidor
	AttrComp->SetAttributeBaseValue(CoinsTag, PlayerCoins - TotalPrice);

	// Concede o item
	InvComp->ServerAddItem(const_cast<USBItemDefinition*>(ItemDef), Quantity);

	UE_LOG(LogTemp, Log, TEXT("USBMerchantComponent::ServerBuyItem: Success - Player bought %d x %s for %d coins"), 
		Quantity, *ItemDef->DisplayName.ToString(), TotalPrice);
}

bool USBMerchantComponent::ServerSellItem_Validate(APawn* PlayerPawn, USBItemInstance* ItemInstance, int32 Quantity)
{
	return true;
}

void USBMerchantComponent::ServerSellItem_Implementation(APawn* PlayerPawn, USBItemInstance* ItemInstance, int32 Quantity)
{
	if (!PlayerPawn || !ItemInstance || Quantity <= 0) return;

	// 1. Validação de Proximidade Física
	float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation());
	if (Dist > 400.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("USBMerchantComponent::ServerSellItem: Rejected - Player too far (%.1f)"), Dist);
		return;
	}

	// 2. Validação da Existência do Item
	const USBItemDefinition* ItemDef = ItemInstance->ItemDef;
	if (!ItemDef) return;

	// 3. Validação de Posse de Itens no Inventário
	USBInventoryComponent* InvComp = PlayerPawn->FindComponentByClass<USBInventoryComponent>();
	if (!InvComp || InvComp->GetTotalItemQuantity(ItemDef) < Quantity)
	{
		UE_LOG(LogTemp, Warning, TEXT("USBMerchantComponent::ServerSellItem: Rejected - Player doesn't have the quantity"));
		return;
	}

	USBAttributeComponent* AttrComp = PlayerPawn->FindComponentByClass<USBAttributeComponent>();
	if (!AttrComp) return;

	// Determina o valor do Payout (Preço Base ou fallback de 10)
	int32 BasePrice = AvailableItems.Contains(ItemDef) ? AvailableItems[ItemDef] : 10;
	int32 TotalPayout = FMath::RoundToInt(BasePrice * SellPriceMultiplier) * Quantity;

	FGameplayTag CoinsTag = FSBGameplayTags::Get().Attribute_Coins;
	float PlayerCoins = AttrComp->GetAttributeValue(CoinsTag);

	// 4. Executa a transação consumindo e adicionando coins
	if (InvComp->ServerRemoveItem(ItemInstance, Quantity))
	{
		AttrComp->SetAttributeBaseValue(CoinsTag, PlayerCoins + TotalPayout);
		UE_LOG(LogTemp, Log, TEXT("USBMerchantComponent::ServerSellItem: Success - Player sold %d x %s for %d coins"), 
			Quantity, *ItemDef->DisplayName.ToString(), TotalPayout);
	}
}
