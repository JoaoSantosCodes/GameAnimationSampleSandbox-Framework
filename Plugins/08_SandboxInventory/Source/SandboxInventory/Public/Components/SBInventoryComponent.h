// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/NetSerialization.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Interfaces/SBEquippableInterface.h"
#include "Interfaces/SBComponentInterface.h"
#include "Interfaces/SBSaveInterface.h"
#include "Interfaces/SBDebugInterface.h"
#include "Interfaces/SBInventoryComponentInterface.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemFragment.h"
#include "Items/SBItemFragment_Equippable.h"
#include "Subsystems/SBEventPayloads.h"
#include "SBInventoryComponent.generated.h"

class USBItemDefinition;
class USBInventoryComponent;

UCLASS(BlueprintType)
class SANDBOXINVENTORY_API USBInventorySlotUpdatedEventPayload : public USBInventoryEventPayload
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 StackCount = 0;
};

UCLASS(BlueprintType)
class SANDBOXINVENTORY_API USBItemEquipPayload : public UObject, public ISBEquipEventPayloadInterface
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<USBItemInstance> ItemInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<const USBItemFragment> EquippableFragment = nullptr;

	// ISBEquipEventPayloadInterface
	virtual UObject* GetEquippableFragment_Implementation() const override
	{
		return const_cast<USBItemFragment*>(EquippableFragment.Get());
	}

	virtual UObject* GetItemInstance_Implementation() const override
	{
		return ItemInstance;
	}
};

USTRUCT(BlueprintType)
struct FSBInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<const USBItemDefinition> ItemDef = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 StackCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FGameplayTagContainer DynamicTags;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	float Durability = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 UpgradeLevel = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<USBItemInstance> Instance = nullptr;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FSBInventoryEntry> : public TStructOpsTypeTraitsBase2<FSBInventoryEntry>
{
	enum
	{
		WithNetSerializer = true,
	};
};

USTRUCT(BlueprintType)
struct FSBInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FSBInventoryEntry> Entries;

	UPROPERTY(Transient)
	TWeakObjectPtr<USBInventoryComponent> OwnerComponent = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FSBInventoryEntry, FSBInventoryList>(Entries, DeltaParms, *this);
	}

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
};

template<>
struct TStructOpsTypeTraits<FSBInventoryList> : public TStructOpsTypeTraitsBase2<FSBInventoryList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

USTRUCT()
struct FSBSavedInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FString ItemDefinitionPath;

	UPROPERTY(SaveGame)
	int32 StackCount = 0;

	UPROPERTY(SaveGame)
	FGameplayTagContainer DynamicTags;

	UPROPERTY(SaveGame)
	float Durability = 100.0f;

	UPROPERTY(SaveGame)
	int32 UpgradeLevel = 0;
};

USTRUCT()
struct FSBSavedInventoryList
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FSBSavedInventorySlot> Slots;
};

USTRUCT()
struct FSBPendingInventoryActivation
{
	GENERATED_BODY()

	int32 ReplicationID = -1;
	float QueueTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FSBStartingItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<USBItemDefinition> ItemDef = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 Quantity = 1;
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBInventoryComponent : public UGameFrameworkComponent, public ISBComponentInterface, public ISBSaveInterface, public ISBDebugInterface, public ISBInventoryComponentInterface
{
	GENERATED_BODY()

public:
	USBInventoryComponent();

	// ISBDebugInterface
	virtual void GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const override;

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {} // Intentionally empty
	virtual void OnPreInitialize_Implementation() override {} // Intentionally empty
	virtual void OnInitialize_Implementation() override;
	// Cacheamento e inscricao em componentes irmaos vive aqui, conforme o manifesto:
	// "OnPostInitialize: Consulta e cacheamento de outros componentes locais".
	virtual void OnPostInitialize_Implementation() override;
	virtual void OnReady_Implementation() override {} // Intentionally empty
	virtual void OnShutdown_Implementation() override {} // Intentionally empty

	// ISBSaveInterface
	virtual bool SaveComponentData_Implementation(UObject* SavePayload) override;
	virtual bool LoadComponentData_Implementation(UObject* SavePayload) override;
	virtual int32 GetSavePriority_Implementation() const override { return 50; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	USBItemInstance* ServerAddItem(USBItemDefinition* ItemDef, int32 Quantity);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	bool ServerRemoveItem(USBItemInstance* ItemInstance, int32 Quantity);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	bool ServerConsumeItemQuantity(const USBItemDefinition* ItemDef, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetTotalItemQuantity(const USBItemDefinition* ItemDef) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void ServerEquipItem(USBItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void ServerUnequipItem(USBItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<USBItemInstance*> GetAllItems() const;

	const FSBInventoryList& GetInventoryList() const { return InventoryList; }

	void OnEntryReplicated(int32 ReplicationID);

	/** Sincroniza e marca dirty as alterações de tags ou quantidades de um ItemInstance no servidor */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void MarkItemInstanceUpdated(USBItemInstance* ItemInstance);

	// ISBInventoryComponentInterface — entrada desacoplada para plugins irmãos
	virtual void NotifyItemInstanceUpdated_Implementation(UObject* ItemInstance) override;
	virtual void GetInventoryDisplayEntries_Implementation(TArray<FSBInventoryDisplayEntry>& OutEntries) override;

	/** Cor do slot para um item sem icone, tirada do fragmento de raridade. */
	FLinearColor RarityColorFor(const class USBItemDefinition* Def) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	bool ServerTransferItem(USBInventoryComponent* TargetInventory, USBItemInstance* ItemInstance, int32 Quantity);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Inventory")
	bool bAutoEquipBetterLoot = false;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float CalculateEffectiveDefense(const USBItemInstance* ItemInstance) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	bool ServerAutoEquipBestArmor(FGameplayTag SlotTag);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void ServerAutoEquipBestArmorAllSlots();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void DeactivateArmorModifiers(USBItemInstance* ItemInstance);

protected:
	/**
	 * Kit inicial: itens concedidos ao dono quando o inventario entra em jogo.
	 *
	 * So o servidor concede. Existe porque todo jogo com inventario precisa disto no primeiro
	 * dia, e sem ele o unico caminho era escrever grafo de Blueprint para cada personagem.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TArray<FSBStartingItem> StartingItems;

protected:
	virtual void BeginPlay() override;

	/** Concede o kit inicial. Chamado em BeginPlay, apenas no servidor. */
	void GrantStartingItems();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleOwnerAttributeChanged(FGameplayTag AttributeTag, float NewValue, float OldValue, AActor* Instigator);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	FSBInventoryList InventoryList;

	// Configurável via Data Asset - classe do fragmento equipável
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Fragments")
	TSubclassOf<USBItemFragment_Equippable> EquippableFragmentClass;

private:
	FDelegateHandle QuestRewardsHandle;
	void HandleQuestRewardsClaimed(FGameplayTag EventTag, UObject* Payload);

	UPROPERTY(Transient)
	TArray<FSBPendingInventoryActivation> PendingActivationSlots;

	class USBEventSubsystem* GetEventSubsystem() const;
	void PublishSlotUpdate(USBItemInstance* Instance, int32 StackCount);
	void RecalculateInventoryWeight();

	UFUNCTION()
	void RestoreEquippedItems();
};
