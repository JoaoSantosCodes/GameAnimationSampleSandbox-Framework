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
#include "Items/SBItemInstance.h"
#include "Items/SBItemFragment.h"
#include "SBInventoryComponent.generated.h"

class USBItemDefinition;
class USBInventoryComponent;

UCLASS(BlueprintType)
class SANDBOXINVENTORY_API USBInventorySlotUpdatedEventPayload : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<USBItemInstance> ItemInstance = nullptr;

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
	TObjectPtr<USBItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 StackCount = 0;
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

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBInventoryComponent : public UGameFrameworkComponent, public ISBComponentInterface, public ISBSaveInterface, public ISBDebugInterface
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
	virtual void OnPostInitialize_Implementation() override {} // Intentionally empty (ready for future UI registration)
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
	void ServerEquipItem(USBItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void ServerUnequipItem(USBItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<USBItemInstance*> GetAllItems() const;

	void OnEntryReplicated(int32 ReplicationID);

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	FSBInventoryList InventoryList;

private:
	UPROPERTY(Transient)
	TArray<FSBPendingInventoryActivation> PendingActivationSlots;

	class USBEventSubsystem* GetEventSubsystem() const;
	void PublishSlotUpdate(USBItemInstance* Instance, int32 StackCount);
};
