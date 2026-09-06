#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SBComponentInterface.h"
#include "DataAssets/SBCraftingRecipeDataAsset.h"
#include "SBCraftingComponent.generated.h"

class USBInventoryComponent;
class USBStateComponent;
class USBItemInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBOnCraftingCompleted, const USBCraftingRecipeDataAsset*, Recipe, USBItemInstance*, CreatedItem, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnCraftingFailed, const USBCraftingRecipeDataAsset*, Recipe, FString, Reason);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBCraftingComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBCraftingComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	virtual void InitializeComponent_Implementation();
	virtual void ResetComponent_Implementation();

	// Valida se o personagem possui todos os insumos e pré-requisitos para fabricar a receita
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Crafting")
	bool CanCraftRecipe(const USBCraftingRecipeDataAsset* Recipe) const;

	// Executa a confecção da receita de forma autoritativa no servidor
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sandbox|Crafting")
	bool ServerCraftRecipe(const USBCraftingRecipeDataAsset* Recipe);

	// Executa o desmantelamento de um item de forma autoritativa no servidor
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sandbox|Crafting")
	bool ServerSalvageItem(USBItemInstance* ItemInstance, int32 Quantity);

	// Executa o reparo de um item de forma autoritativa no servidor se estiver perto de uma bancada
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sandbox|Crafting")
	bool ServerRepairItem(USBItemInstance* ItemInstance);

	// Executa o upgrade de um item de forma autoritativa no servidor consumindo materiais e moedas
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sandbox|Crafting")
	bool ServerUpgradeItem(USBItemInstance* ItemInstance);

	// Delegates para subscrição Blueprint/C++
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Crafting")
	FSBOnCraftingCompleted OnCraftingCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Crafting")
	FSBOnCraftingFailed OnCraftingFailed;

	// Delegates nativos C++ para Salvage
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FSBOnSalvagingCompleted, const USBItemDefinition* /*OriginalItemDef*/, int32 /*Quantity*/, const TArray<USBItemInstance*>& /*GainedItems*/);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FSBOnSalvagingFailed, const USBItemDefinition* /*OriginalItemDef*/, const FString& /*Reason*/);

	FSBOnSalvagingCompleted OnSalvagingCompleted;
	FSBOnSalvagingFailed OnSalvagingFailed;

protected:
	UPROPERTY(Transient)
	TObjectPtr<USBInventoryComponent> CachedInventoryComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USBStateComponent> CachedStateComponent = nullptr;

private:
	class USBEventSubsystem* GetEventSubsystem() const;
};
