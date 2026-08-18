#include "Components/SBInventoryComponent.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment_Equippable.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/ActorChannel.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "TimerManager.h"

void FSBInventoryList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	if (OwnerComponent.IsValid())
	{
		for (int32 Index : AddedIndices)
		{
			OwnerComponent->OnEntryReplicated(Entries[Index].ReplicationID);
		}
	}
}

void FSBInventoryList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	if (OwnerComponent.IsValid())
	{
		for (int32 Index : ChangedIndices)
		{
			OwnerComponent->OnEntryReplicated(Entries[Index].ReplicationID);
		}
	}
}

USBInventoryComponent::USBInventoryComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bReplicateUsingRegisteredSubObjectList = false;
	SetIsReplicatedByDefault(true);
}

void USBInventoryComponent::OnInitialize_Implementation()
{
	InventoryList.OwnerComponent = this;
}

void USBInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBInventoryComponent, InventoryList);
}

bool USBInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (const FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance)
		{
			bWroteSomething |= Channel->ReplicateSubobject(Entry.Instance, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

void USBInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	for (int32 Index = PendingActivationSlots.Num() - 1; Index >= 0; --Index)
	{
		const FSBPendingInventoryActivation& Pending = PendingActivationSlots[Index];

		const FSBInventoryEntry* FoundEntry = nullptr;
		for (const FSBInventoryEntry& Entry : InventoryList.Entries)
		{
			if (Entry.ReplicationID == Pending.ReplicationID)
			{
				FoundEntry = &Entry;
				break;
			}
		}

		if (FoundEntry)
		{
			if (FoundEntry->Instance && FoundEntry->Instance->ItemDef)
			{
				PublishSlotUpdate(FoundEntry->Instance, FoundEntry->StackCount);
				PendingActivationSlots.RemoveAt(Index);
			}
			else if (CurrentTime - Pending.QueueTime > 2.0f)
			{
				UE_LOG(LogTemp, Warning, TEXT("LogSandbox: Warning: Slot activation timed out for ReplicationID %d"), Pending.ReplicationID);
				PendingActivationSlots.RemoveAt(Index);
			}
		}
		else
		{
			PendingActivationSlots.RemoveAt(Index);
		}
	}
}

USBItemInstance* USBInventoryComponent::ServerAddItem(USBItemDefinition* ItemDef, int32 Quantity)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !ItemDef || Quantity <= 0)
	{
		return nullptr;
	}

	USBItemInstance* ResultInstance = nullptr;
	int32 RemainingQuantity = Quantity;

	if (ItemDef->MaxStackCount > 1)
	{
		for (FSBInventoryEntry& Entry : InventoryList.Entries)
		{
			if (Entry.Instance && Entry.Instance->ItemDef == ItemDef)
			{
				int32 SpaceLeft = ItemDef->MaxStackCount - Entry.StackCount;
				if (SpaceLeft > 0)
				{
					int32 AmountToStack = FMath::Min(RemainingQuantity, SpaceLeft);
					Entry.StackCount += AmountToStack;
					Entry.Instance->StackCount = Entry.StackCount;
					
					InventoryList.MarkItemDirty(Entry);
					
					RemainingQuantity -= AmountToStack;
					ResultInstance = Entry.Instance;

					PublishSlotUpdate(Entry.Instance, Entry.StackCount);

					if (RemainingQuantity <= 0)
					{
						break;
					}
				}
			}
		}
	}

	while (RemainingQuantity > 0)
	{
		int32 AmountToCreate = FMath::Min(RemainingQuantity, ItemDef->MaxStackCount > 1 ? ItemDef->MaxStackCount : 1);
		
		USBItemInstance* NewInstance = NewObject<USBItemInstance>(this);
		NewInstance->ItemDef = ItemDef;
		NewInstance->StackCount = AmountToCreate;

		FSBInventoryEntry NewEntry;
		NewEntry.Instance = NewInstance;
		NewEntry.StackCount = AmountToCreate;

		InventoryList.Entries.Add(NewEntry);
		InventoryList.MarkArrayDirty();

		RemainingQuantity -= AmountToCreate;
		ResultInstance = NewInstance;

		PublishSlotUpdate(NewInstance, AmountToCreate);

		if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
		{
			USBInventoryEventPayload* AddPayload = NewObject<USBInventoryEventPayload>(this);
			AddPayload->TargetPawn = Cast<APawn>(GetOwner());
			AddPayload->ItemInstance = NewInstance;
			EventSubsystem->PublishEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.ItemAdded")), AddPayload);
		}
	}

	return ResultInstance;
}

bool USBInventoryComponent::ServerRemoveItem(USBItemInstance* ItemInstance, int32 Quantity)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !ItemInstance || Quantity <= 0)
	{
		return false;
	}

	for (int32 Index = 0; Index < InventoryList.Entries.Num(); ++Index)
	{
		FSBInventoryEntry& Entry = InventoryList.Entries[Index];
		if (Entry.Instance == ItemInstance)
		{
			int32 AmountToRemove = FMath::Min(Quantity, Entry.StackCount);
			Entry.StackCount -= AmountToRemove;

			if (Entry.StackCount <= 0)
			{
				USBItemInstance* RemovedInstance = Entry.Instance;
				InventoryList.Entries.RemoveAt(Index);
				InventoryList.MarkArrayDirty();

				if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
				{
					USBInventoryEventPayload* RemovePayload = NewObject<USBInventoryEventPayload>(this);
					RemovePayload->TargetPawn = Cast<APawn>(GetOwner());
					RemovePayload->ItemInstance = RemovedInstance;
					EventSubsystem->PublishEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.ItemRemoved")), RemovePayload);
				}

				PublishSlotUpdate(nullptr, 0);
			}
			else
			{
				Entry.Instance->StackCount = Entry.StackCount;
				InventoryList.MarkItemDirty(Entry);
				PublishSlotUpdate(Entry.Instance, Entry.StackCount);
			}

			return true;
		}
	}

	return false;
}

void USBInventoryComponent::ServerEquipItem(USBItemInstance* ItemInstance)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !ItemInstance)
	{
		return;
	}

	TSubclassOf<USBItemFragment> FragmentClass = EquippableFragmentClass;
	if (!FragmentClass)
	{
		FragmentClass = USBItemFragment_Equippable::StaticClass();
	}

	const USBItemFragment* RawFragment = ItemInstance->FindFragmentByClass(FragmentClass);
	if (RawFragment)
	{
		FGameplayTag EquippedTag = FGameplayTag::RequestGameplayTag(TEXT("State.Item.Equipped"), false);
		if (EquippedTag.IsValid())
		{
			ItemInstance->DynamicTags.AddTag(EquippedTag);
		}

		if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
		{
			USBItemEquipPayload* Payload = NewObject<USBItemEquipPayload>(this);
			Payload->TargetPawn = Cast<APawn>(GetOwner());
			Payload->ItemInstance = ItemInstance;
			Payload->EquippableFragment = RawFragment;
			
			FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.ItemEquipped"));
			EventSubsystem->PublishEvent(EquipTag, Payload);
		}
	}
}

void USBInventoryComponent::ServerUnequipItem(USBItemInstance* ItemInstance)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !ItemInstance)
	{
		return;
	}

	TSubclassOf<USBItemFragment> FragmentClass = EquippableFragmentClass;
	if (!FragmentClass)
	{
		FragmentClass = USBItemFragment_Equippable::StaticClass();
	}

	const USBItemFragment* RawFragment = ItemInstance->FindFragmentByClass(FragmentClass);
	if (RawFragment)
	{
		FGameplayTag EquippedTag = FGameplayTag::RequestGameplayTag(TEXT("State.Item.Equipped"), false);
		if (EquippedTag.IsValid())
		{
			ItemInstance->DynamicTags.RemoveTag(EquippedTag);
		}

		if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
		{
			USBItemEquipPayload* Payload = NewObject<USBItemEquipPayload>(this);
			Payload->TargetPawn = Cast<APawn>(GetOwner());
			Payload->ItemInstance = ItemInstance;
			Payload->EquippableFragment = RawFragment;
			
			FGameplayTag UnequipTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.ItemUnequipped"));
			EventSubsystem->PublishEvent(UnequipTag, Payload);
		}
	}
}

TArray<USBItemInstance*> USBInventoryComponent::GetAllItems() const
{
	TArray<USBItemInstance*> Items;
	for (const FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance)
		{
			Items.Add(Entry.Instance);
		}
	}
	return Items;
}

void USBInventoryComponent::OnEntryReplicated(int32 ReplicationID)
{
	for (const FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ReplicationID == ReplicationID)
		{
			if (Entry.Instance && Entry.Instance->ItemDef)
			{
				PublishSlotUpdate(Entry.Instance, Entry.StackCount);
			}
			else
			{
				FSBPendingInventoryActivation Pending;
				Pending.ReplicationID = ReplicationID;
				Pending.QueueTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
				PendingActivationSlots.Add(Pending);
			}
			break;
		}
	}
}

USBEventSubsystem* USBInventoryComponent::GetEventSubsystem() const
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<USBEventSubsystem>() : nullptr;
}

void USBInventoryComponent::PublishSlotUpdate(USBItemInstance* Instance, int32 StackCount)
{
	if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
	{
		USBInventorySlotUpdatedEventPayload* Payload = NewObject<USBInventorySlotUpdatedEventPayload>(this);
		Payload->TargetPawn = Cast<APawn>(GetOwner());
		Payload->ItemInstance = Instance;
		Payload->StackCount = StackCount;
		
		FGameplayTag UpdatedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.SlotUpdated"));
		EventSubsystem->PublishEvent(UpdatedTag, Payload);
	}
}

bool USBInventoryComponent::SaveComponentData_Implementation(UObject* SavePayload)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (Payload)
	{
		FSBSavedInventoryList SavedList;
		for (const FSBInventoryEntry& Entry : InventoryList.Entries)
		{
			if (Entry.Instance && Entry.Instance->ItemDef)
			{
				FSBSavedInventorySlot SavedSlot;
				SavedSlot.ItemDefinitionPath = Entry.Instance->ItemDef->GetPathName();
				SavedSlot.StackCount = Entry.StackCount;
				SavedSlot.DynamicTags = Entry.Instance->DynamicTags;
				SavedList.Slots.Add(SavedSlot);
			}
		}

		TArray<uint8> BinaryData;
		FMemoryWriter Writer(BinaryData);
		FObjectAndNameAsStringProxyArchive Archive(Writer, true);
		Archive.ArIsSaveGame = true;

		FSBSavedInventoryList::StaticStruct()->SerializeItem(Archive, &SavedList, nullptr);

		Payload->WriteBinaryData(GetPathName(), BinaryData);
		return true;
	}
	return false;
}

bool USBInventoryComponent::LoadComponentData_Implementation(UObject* SavePayload)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return false;
	}

	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (Payload)
	{
		TArray<uint8> BinaryData;
		if (Payload->ReadBinaryData(GetPathName(), BinaryData) && BinaryData.Num() > 0)
		{
			FSBSavedInventoryList SavedList;
			FMemoryReader Reader(BinaryData);
			FObjectAndNameAsStringProxyArchive Archive(Reader, true);
			Archive.ArIsSaveGame = true;

			FSBSavedInventoryList::StaticStruct()->SerializeItem(Archive, &SavedList, nullptr);

			// Esvazia os slots atuais de forma autoritativa
			InventoryList.Entries.Empty();
			InventoryList.MarkArrayDirty();

			// Reconstitui o inventário via ServerAddItem para manter callbacks e rede consistentes
			for (const FSBSavedInventorySlot& SavedSlot : SavedList.Slots)
			{
				USBItemDefinition* ItemDef = Cast<USBItemDefinition>(StaticLoadObject(USBItemDefinition::StaticClass(), nullptr, *SavedSlot.ItemDefinitionPath));
				if (ItemDef)
				{
					USBItemInstance* NewInstance = ServerAddItem(ItemDef, SavedSlot.StackCount);
					if (NewInstance)
					{
						NewInstance->DynamicTags = SavedSlot.DynamicTags;
					}
				}
			}

			if (GetWorld())
			{
				GetWorld()->GetTimerManager().SetTimerForNextTick(this, &USBInventoryComponent::RestoreEquippedItems);
			}

			return true;
		}
	}
	return false;
}

void USBInventoryComponent::RestoreEquippedItems()
{
	FGameplayTag EquippedTag = FGameplayTag::RequestGameplayTag(TEXT("State.Item.Equipped"), false);
	if (!EquippedTag.IsValid())
	{
		return;
	}

	TArray<USBItemInstance*> ItemsToEquip;
	for (const FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance && Entry.Instance->DynamicTags.HasTag(EquippedTag))
		{
			ItemsToEquip.Add(Entry.Instance);
		}
	}

	for (USBItemInstance* Item : ItemsToEquip)
	{
		ServerEquipItem(Item);
	}
}

void USBInventoryComponent::GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const
{
	FSBDebugLine Header;
	Header.Label = GetClass()->GetName();
	Header.bIsHeader = true;
	OutDebugLines.Add(Header);

	FSBDebugLine SlotsHeader;
	SlotsHeader.Label = FString::Printf(TEXT("Inventory Slots (%d)"), InventoryList.Entries.Num());
	SlotsHeader.bIsHeader = true;
	OutDebugLines.Add(SlotsHeader);

	for (const FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance)
		{
			FString TagsStr = Entry.Instance->DynamicTags.ToStringSimple();
			FSBDebugLine Line;
			Line.Label = Entry.Instance->ItemDef ? Entry.Instance->ItemDef->GetName() : TEXT("Unknown Item");
			Line.Value = FString::Printf(TEXT("Count: %d | Tags: %s"), Entry.StackCount, TagsStr.IsEmpty() ? TEXT("None") : *TagsStr);
			OutDebugLines.Add(Line);
		}
	}
}
