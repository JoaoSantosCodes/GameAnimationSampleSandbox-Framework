#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SBMerchantComponent.generated.h"

class USBItemDefinition;
class USBItemInstance;
class APawn;

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBMerchantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USBMerchantComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Merchant")
	TMap<TObjectPtr<const USBItemDefinition>, int32> AvailableItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Merchant")
	float BuyPriceMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Merchant")
	float SellPriceMultiplier;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Merchant")
	void ServerBuyItem(APawn* PlayerPawn, const USBItemDefinition* ItemDef, int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Merchant")
	void ServerSellItem(APawn* PlayerPawn, USBItemInstance* ItemInstance, int32 Quantity);
};
