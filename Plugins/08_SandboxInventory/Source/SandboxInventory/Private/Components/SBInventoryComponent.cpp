// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBInventoryComponent.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment_Equippable.h"
#include "Items/SBItemFragment_Armor.h"
#include "Items/SBItemFragment_Upgrade.h"
#include "Items/SBItemFragment_Durability.h"
#include "Items/SBItemFragment_Weight.h"
#include "Items/SBItemFragment_Rarity.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "SBGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/ActorChannel.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "TimerManager.h"

namespace
{
	/**
	 * Cor do slot quando o item nao tem icone. As tags de raridade sao as unicas registradas
	 * no framework hoje; qualquer outra cai no cinza neutro.
	 */
	FLinearColor RarityColorFromTag(const FGameplayTag& Tag)
	{
		static const FName Comum(TEXT("Loot.Rarity.Common"));
		static const FName Incomum(TEXT("Loot.Rarity.Uncommon"));
		static const FName Raro(TEXT("Loot.Rarity.Rare"));
		static const FName Epico(TEXT("Loot.Rarity.Epic"));
		static const FName Lendario(TEXT("Loot.Rarity.Legendary"));

		const FName Nome = Tag.GetTagName();
		if (Nome == Incomum)  return FLinearColor(0.35f, 0.75f, 0.35f, 1.0f);
		if (Nome == Raro)     return FLinearColor(0.30f, 0.55f, 0.95f, 1.0f);
		if (Nome == Epico)    return FLinearColor(0.65f, 0.35f, 0.90f, 1.0f);
		if (Nome == Lendario) return FLinearColor(0.95f, 0.65f, 0.20f, 1.0f);
		if (Nome == Comum)    return FLinearColor(0.60f, 0.60f, 0.60f, 1.0f);
		return FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
	}
}

bool FSBInventoryEntry::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	UObject* TempObj = const_cast<USBItemDefinition*>(ItemDef.Get());
	bOutSuccess = Map->SerializeObject(Ar, USBItemDefinition::StaticClass(), TempObj);
	ItemDef = Cast<const USBItemDefinition>(TempObj);

	uint32 StackCountUint = static_cast<uint32>(StackCount);
	Ar.SerializeIntPacked(StackCountUint);
	StackCount = static_cast<int32>(StackCountUint);

	DynamicTags.NetSerialize(Ar, Map, bOutSuccess);

	Ar << Durability;
	Ar << UpgradeLevel;

	bOutSuccess = true;
	return true;
}

void FSBInventoryList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	if (OwnerComponent.IsValid())
	{
		for (int32 Index : AddedIndices)
		{
			FSBInventoryEntry& Entry = Entries[Index];
			if (!Entry.Instance && Entry.ItemDef)
			{
				Entry.Instance = NewObject<USBItemInstance>(OwnerComponent.Get());
				Entry.Instance->ItemDef = Entry.ItemDef;
				Entry.Instance->StackCount = Entry.StackCount;
				Entry.Instance->DynamicTags = Entry.DynamicTags;
				Entry.Instance->Durability = Entry.Durability;
				Entry.Instance->UpgradeLevel = Entry.UpgradeLevel;
			}
			OwnerComponent->OnEntryReplicated(Entry.ReplicationID);
		}
	}
}

void FSBInventoryList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	if (OwnerComponent.IsValid())
	{
		for (int32 Index : ChangedIndices)
		{
			FSBInventoryEntry& Entry = Entries[Index];
			if (Entry.ItemDef)
			{
				if (!Entry.Instance)
				{
					Entry.Instance = NewObject<USBItemInstance>(OwnerComponent.Get());
				}
				Entry.Instance->ItemDef = Entry.ItemDef;
				Entry.Instance->StackCount = Entry.StackCount;
				Entry.Instance->DynamicTags = Entry.DynamicTags;
				Entry.Instance->Durability = Entry.Durability;
				Entry.Instance->UpgradeLevel = Entry.UpgradeLevel;
			}
			else
			{
				Entry.Instance = nullptr;
			}
			OwnerComponent->OnEntryReplicated(Entry.ReplicationID);
		}
	}
}

void FSBInventoryList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	if (OwnerComponent.IsValid())
	{
		for (int32 Index : RemovedIndices)
		{
			FSBInventoryEntry& Entry = Entries[Index];
			if (Entry.Instance)
			{
				Entry.Instance = nullptr;
			}
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
	InventoryList.OwnerComponent = this;
}

void USBInventoryComponent::OnInitialize_Implementation()
{
	InventoryList.OwnerComponent = this;

	// Registra os atributos de peso de forma automatizada no AttributeComponent do Owner
	AActor* Owner = GetOwner();
	if (Owner)
	{
		USBAttributeComponent* AttrComp = Owner->FindComponentByClass<USBAttributeComponent>();
		if (AttrComp)
		{
			FGameplayTag WeightTag = FSBGameplayTags::Get().Attribute_Weight;
			FGameplayTag MaxWeightTag = FSBGameplayTags::Get().Attribute_MaxWeight;

			// Registra Weight (Inicial = 0)
			FSBAttribute WeightAttr;
			WeightAttr.BaseValue = 0.0f;
			WeightAttr.CurrentValue = 0.0f;
			WeightAttr.MaxValue = 9999.0f;
			WeightAttr.MinValue = 0.0f;
			AttrComp->RegisterAttribute(WeightTag, WeightAttr);

			// Registra MaxWeight (Inicial = 100.f)
			FSBAttribute MaxWeightAttr;
			MaxWeightAttr.BaseValue = 100.0f;
			MaxWeightAttr.CurrentValue = 100.0f;
			MaxWeightAttr.MaxValue = 9999.0f;
			MaxWeightAttr.MinValue = 0.0f;
			AttrComp->RegisterAttribute(MaxWeightTag, MaxWeightAttr);
		}
	}
}

void USBInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBInventoryComponent, InventoryList);
	DOREPLIFETIME(USBInventoryComponent, bAutoEquipBetterLoot);
}

bool USBInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// Replicação individual do ItemInstance desativada. As propriedades são empacotadas
	// via FSBInventoryEntry::NetSerialize para reduzir payload e overhead de canais (Fase 37).

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

		// Inicializa Durabilidade se possuir o fragmento correspondente
		if (const USBItemFragment_Durability* DurabilityFragment = Cast<USBItemFragment_Durability>(NewInstance->FindFragmentByClass(USBItemFragment_Durability::StaticClass())))
		{
			NewInstance->Durability = DurabilityFragment->InitialDurability;
		}

		FSBInventoryEntry NewEntry;
		NewEntry.Instance = NewInstance;
		NewEntry.ItemDef = ItemDef;
		NewEntry.StackCount = AmountToCreate;
		NewEntry.DynamicTags = NewInstance->DynamicTags;
		NewEntry.Durability = NewInstance->Durability;

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
			EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Inventory_ItemAdded, AddPayload);
		}
	}

	RecalculateInventoryWeight();

	if (bAutoEquipBetterLoot && ResultInstance)
	{
		const USBItemFragment_Armor* ArmorFragment = Cast<USBItemFragment_Armor>(ResultInstance->FindFragmentByClass(USBItemFragment_Armor::StaticClass()));
		if (ArmorFragment)
		{
			ServerAutoEquipBestArmor(ArmorFragment->EquipmentSlotTag);
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
					EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Inventory_ItemRemoved, RemovePayload);
				}

				PublishSlotUpdate(nullptr, 0);
			}
			else
			{
				Entry.Instance->StackCount = Entry.StackCount;
				InventoryList.MarkItemDirty(Entry);
				PublishSlotUpdate(Entry.Instance, Entry.StackCount);
			}

			RecalculateInventoryWeight();

			return true;
		}
	}

	return false;
}

int32 USBInventoryComponent::GetTotalItemQuantity(const USBItemDefinition* ItemDef) const
{
	if (!ItemDef)
	{
		return 0;
	}

	int32 Total = 0;
	for (const FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance && Entry.Instance->ItemDef == ItemDef)
		{
			Total += Entry.StackCount;
		}
	}
	return Total;
}

bool USBInventoryComponent::ServerConsumeItemQuantity(const USBItemDefinition* ItemDef, int32 Quantity)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !ItemDef || Quantity <= 0)
	{
		return false;
	}

	if (GetTotalItemQuantity(ItemDef) < Quantity)
	{
		return false;
	}

	int32 RemainingToConsume = Quantity;
	for (int32 Index = InventoryList.Entries.Num() - 1; Index >= 0 && RemainingToConsume > 0; --Index)
	{
		FSBInventoryEntry& Entry = InventoryList.Entries[Index];
		if (Entry.Instance && Entry.Instance->ItemDef == ItemDef)
		{
			int32 AmountFromThisSlot = FMath::Min(RemainingToConsume, Entry.StackCount);
			RemainingToConsume -= AmountFromThisSlot;
			ServerRemoveItem(Entry.Instance, AmountFromThisSlot);
		}
	}

	return RemainingToConsume == 0;
}

void USBInventoryComponent::ServerEquipItem(USBItemInstance* ItemInstance)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !ItemInstance)
	{
		return;
	}

	// 1. Suporte a Armaduras e Modificadores de Atributos
	const USBItemFragment_Armor* ArmorFragment = Cast<USBItemFragment_Armor>(ItemInstance->FindFragmentByClass(USBItemFragment_Armor::StaticClass()));
	if (ArmorFragment)
	{
		// Substituição Automática (Slot Ejection): desequipa qualquer item previamente equipado no mesmo slot
		if (ArmorFragment->EquipmentSlotTag.IsValid())
		{
			for (const FSBInventoryEntry& Entry : InventoryList.Entries)
			{
				if (Entry.Instance && Entry.Instance != ItemInstance && Entry.Instance->DynamicTags.HasTag(ArmorFragment->EquipmentSlotTag))
				{
					ServerUnequipItem(Entry.Instance);
				}
			}
			ItemInstance->DynamicTags.AddTag(ArmorFragment->EquipmentSlotTag);
		}

		// Limpa qualquer modificador existente da mesma origem para evitar duplicação antes de aplicar
		DeactivateArmorModifiers(ItemInstance);

		// Concede modificadores no USBAttributeComponent (apenas se tiver durabilidade > 0)
		if (ItemInstance->Durability > 0.0f)
		{
			AActor* Owner = GetOwner();
			USBAttributeComponent* AttrComp = Owner ? Owner->FindComponentByClass<USBAttributeComponent>() : nullptr;
			if (AttrComp)
			{
				float UpgradeMultiplier = 1.0f;
				const USBItemFragment* FoundUpgradeFrag = ItemInstance->FindFragmentByClass(USBItemFragment_Upgrade::StaticClass());
				if (const USBItemFragment_Upgrade* UpgradeFrag = Cast<USBItemFragment_Upgrade>(FoundUpgradeFrag))
				{
					for (int32 i = 0; i < ItemInstance->UpgradeLevel; ++i)
					{
						if (UpgradeFrag->UpgradeLevels.IsValidIndex(i))
						{
							UpgradeMultiplier += UpgradeFrag->UpgradeLevels[i].StatMultiplierBonus;
						}
					}
				}

				FGameplayTag SourceTag = ArmorFragment->EquipmentSlotTag.IsValid() ? ArmorFragment->EquipmentSlotTag : FSBGameplayTags::Get().State_Item_Equipped;
				for (const FSBItemAttributeModifier& ModConfig : ArmorFragment->ModifiersToGrant)
				{
					if (ModConfig.AttributeTag.IsValid() && ModConfig.Magnitude != 0.0f)
					{
						FSBAttributeModifier NewMod;
						NewMod.SourceTag = SourceTag;
						NewMod.ModifierType = ModConfig.ModifierType;
						NewMod.Magnitude = ModConfig.Magnitude * UpgradeMultiplier;
						NewMod.Duration = 0.0f; // Permanente enquanto equipado
						NewMod.StackCount = 1;
						AttrComp->ApplyModifier(ModConfig.AttributeTag, NewMod);
					}
				}
			}
		}

		FGameplayTag EquippedTag = FSBGameplayTags::Get().State_Item_Equipped;
		if (EquippedTag.IsValid())
		{
			ItemInstance->DynamicTags.AddTag(EquippedTag);
		}

		if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
		{
			USBItemEquipPayload* Payload = NewObject<USBItemEquipPayload>(this);
			Payload->TargetPawn = Cast<APawn>(GetOwner());
			Payload->ItemInstance = ItemInstance;
			Payload->EquippableFragment = ArmorFragment;
			
			EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Inventory_ItemEquipped, Payload);
		}
	}

	// 2. Suporte a Armas / Comportamentos Equipáveis
	TSubclassOf<USBItemFragment> FragmentClass = EquippableFragmentClass;
	if (!FragmentClass)
	{
		FragmentClass = USBItemFragment_Equippable::StaticClass();
	}

	const USBItemFragment* RawFragment = ItemInstance->FindFragmentByClass(FragmentClass);
	if (RawFragment && !ArmorFragment)
	{
		FGameplayTag EquippedTag = FSBGameplayTags::Get().State_Item_Equipped;
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
			
			EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Inventory_ItemEquipped, Payload);
		}
	}

	MarkItemInstanceUpdated(ItemInstance);
}

void USBInventoryComponent::ServerUnequipItem(USBItemInstance* ItemInstance)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !ItemInstance)
	{
		return;
	}

	// 1. Suporte a Armaduras e Modificadores de Atributos
	const USBItemFragment_Armor* ArmorFragment = Cast<USBItemFragment_Armor>(ItemInstance->FindFragmentByClass(USBItemFragment_Armor::StaticClass()));
	if (ArmorFragment)
	{
		// Remove modificadores do USBAttributeComponent
		AActor* Owner = GetOwner();
		USBAttributeComponent* AttrComp = Owner ? Owner->FindComponentByClass<USBAttributeComponent>() : nullptr;
		if (AttrComp)
		{
			FGameplayTag SourceTag = ArmorFragment->EquipmentSlotTag.IsValid() ? ArmorFragment->EquipmentSlotTag : FSBGameplayTags::Get().State_Item_Equipped;
			for (const FSBItemAttributeModifier& ModConfig : ArmorFragment->ModifiersToGrant)
			{
				if (ModConfig.AttributeTag.IsValid())
				{
					AttrComp->RemoveModifiersBySource(ModConfig.AttributeTag, SourceTag);
				}
			}
		}

		if (ArmorFragment->EquipmentSlotTag.IsValid())
		{
			ItemInstance->DynamicTags.RemoveTag(ArmorFragment->EquipmentSlotTag);
		}

		FGameplayTag EquippedTag = FSBGameplayTags::Get().State_Item_Equipped;
		if (EquippedTag.IsValid())
		{
			ItemInstance->DynamicTags.RemoveTag(EquippedTag);
		}

		if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
		{
			USBItemEquipPayload* Payload = NewObject<USBItemEquipPayload>(this);
			Payload->TargetPawn = Cast<APawn>(GetOwner());
			Payload->ItemInstance = ItemInstance;
			Payload->EquippableFragment = ArmorFragment;
			
			EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Inventory_ItemUnequipped, Payload);
		}
	}

	// 2. Suporte a Armas / Comportamentos Equipáveis
	TSubclassOf<USBItemFragment> FragmentClass = EquippableFragmentClass;
	if (!FragmentClass)
	{
		FragmentClass = USBItemFragment_Equippable::StaticClass();
	}

	const USBItemFragment* RawFragment = ItemInstance->FindFragmentByClass(FragmentClass);
	if (RawFragment && !ArmorFragment)
	{
		FGameplayTag EquippedTag = FSBGameplayTags::Get().State_Item_Equipped;
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
			
			EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Inventory_ItemUnequipped, Payload);
		}
	}

	MarkItemInstanceUpdated(ItemInstance);
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
		
		EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Inventory_SlotUpdated, Payload);
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
				SavedSlot.Durability = Entry.Instance->Durability;
				SavedSlot.UpgradeLevel = Entry.Instance->UpgradeLevel;
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
						NewInstance->Durability = SavedSlot.Durability;
						NewInstance->UpgradeLevel = SavedSlot.UpgradeLevel;
						MarkItemInstanceUpdated(NewInstance);
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
	FGameplayTag EquippedTag = FSBGameplayTags::Get().State_Item_Equipped;
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

FLinearColor USBInventoryComponent::RarityColorFor(const USBItemDefinition* Def) const
{
	if (!Def)
	{
		return FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
	}

	const USBItemFragment* Fragmento = Def->FindFragmentByClass(USBItemFragment_Rarity::StaticClass());
	const USBItemFragment_Rarity* Raridade = Cast<USBItemFragment_Rarity>(Fragmento);
	return Raridade ? RarityColorFromTag(Raridade->RarityTag) : FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
}

void USBInventoryComponent::GetInventoryDisplayEntries_Implementation(TArray<FSBInventoryDisplayEntry>& OutEntries)
{
	OutEntries.Reset();

	for (const USBItemInstance* Item : GetAllItems())
	{
		if (!Item || !Item->ItemDef)
		{
			continue;
		}

		FSBInventoryDisplayEntry Entrada;

		// Item sem nome de exibicao ainda precisa aparecer: cair no nome do asset e melhor do
		// que sumir da grade e deixar o jogador achar que perdeu o item.
		Entrada.Name = Item->ItemDef->DisplayName;
		if (Entrada.Name.IsEmpty())
		{
			Entrada.Name = FText::FromString(Item->ItemDef->GetName());
		}

		Entrada.StackCount = Item->StackCount;
		Entrada.Icon = Item->ItemDef->Icon;
		Entrada.RarityColor = RarityColorFor(Item->ItemDef);

		OutEntries.Add(MoveTemp(Entrada));
	}
}

void USBInventoryComponent::NotifyItemInstanceUpdated_Implementation(UObject* ItemInstance)
{
	MarkItemInstanceUpdated(Cast<USBItemInstance>(ItemInstance));
}

void USBInventoryComponent::MarkItemInstanceUpdated(USBItemInstance* ItemInstance)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !ItemInstance)
	{
		return;
	}

	for (FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance == ItemInstance)
		{
			Entry.ItemDef = ItemInstance->ItemDef;
			Entry.StackCount = ItemInstance->StackCount;
			Entry.DynamicTags = ItemInstance->DynamicTags;
			Entry.Durability = ItemInstance->Durability;
			Entry.UpgradeLevel = ItemInstance->UpgradeLevel;
			InventoryList.MarkItemDirty(Entry);
			break;
		}
	}
}

#include "DataAssets/SBQuestDataAsset.h"


void USBInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	USBEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (EventSubsystem && GetOwner() && GetOwner()->HasAuthority())
	{
		QuestRewardsHandle = EventSubsystem->SubscribeToEventNative(
			FSBGameplayTags::Get().Event_Quest_RewardsClaimed,
			ESBEventPriority::Medium,
			FSBNativeEventDelegate::CreateUObject(this, &USBInventoryComponent::HandleQuestRewardsClaimed)
		);
	}

	GrantStartingItems();
}

void USBInventoryComponent::GrantStartingItems()
{
	// So o servidor concede: em cliente isto duplicaria o kit assim que a replicacao chegasse.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (const FSBStartingItem& Inicial : StartingItems)
	{
		if (!Inicial.ItemDef || Inicial.Quantity <= 0)
		{
			continue;
		}

		ServerAddItem(Inicial.ItemDef, Inicial.Quantity);
	}
}

void USBInventoryComponent::OnPostInitialize_Implementation()
{
	// A inscricao no componente de atributos ficava em BeginPlay, que executa no momento do
	// RegisterComponent deste componente. Se o componente de atributos ainda nao existisse
	// naquele instante, a inscricao falhava silenciosamente e a durabilidade nunca era
	// consumida. OnPostInitialize e o gancho que o manifesto reserva para isto: roda depois
	// de todos os componentes existirem.
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (USBAttributeComponent* AttrComp = GetOwner()->FindComponentByClass<USBAttributeComponent>())
		{
			AttrComp->OnAttributeChanged.RemoveDynamic(this, &USBInventoryComponent::HandleOwnerAttributeChanged);
			AttrComp->OnAttributeChanged.AddDynamic(this, &USBInventoryComponent::HandleOwnerAttributeChanged);
		}
	}
}

void USBInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	USBEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (EventSubsystem && QuestRewardsHandle.IsValid())
	{
		EventSubsystem->UnsubscribeFromEventNative(FSBGameplayTags::Get().Event_Quest_RewardsClaimed, QuestRewardsHandle);
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (USBAttributeComponent* AttrComp = GetOwner()->FindComponentByClass<USBAttributeComponent>())
		{
			AttrComp->OnAttributeChanged.RemoveDynamic(this, &USBInventoryComponent::HandleOwnerAttributeChanged);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void USBInventoryComponent::HandleQuestRewardsClaimed(FGameplayTag EventTag, UObject* Payload)
{
	USBQuestRewardsPayload* QuestPayload = Cast<USBQuestRewardsPayload>(Payload);
	if (!QuestPayload || QuestPayload->TargetPawn != GetOwner()) return;

	USBQuestDataAsset* Quest = Cast<USBQuestDataAsset>(QuestPayload->QuestData);
	if (!Quest) return;

	// Adiciona os itens de recompensa da missão no inventário
	for (const FSBQuestReward& Reward : Quest->Rewards)
	{
		if (Reward.ItemDef.IsValid() || !Reward.ItemDef.IsNull())
		{
			USBItemDefinition* LoadedItemDef = Cast<USBItemDefinition>(Reward.ItemDef.LoadSynchronous());
			if (LoadedItemDef)
			{
				ServerAddItem(LoadedItemDef, Reward.Quantity);
			}
		}
	}
}

void USBInventoryComponent::RecalculateInventoryWeight()
{
	float TotalWeight = 0.0f;
	for (const FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemDef && Entry.StackCount > 0)
		{
			float ItemWeight = 0.0f;
			if (const USBItemFragment_Weight* WeightFrag = Cast<USBItemFragment_Weight>(Entry.ItemDef->FindFragmentByClass(USBItemFragment_Weight::StaticClass())))
			{
				ItemWeight = WeightFrag->Weight;
			}
			TotalWeight += ItemWeight * Entry.StackCount;
		}
	}

	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		USBAttributeComponent* AttrComp = Owner->FindComponentByClass<USBAttributeComponent>();
		if (AttrComp)
		{
			FGameplayTag WeightTag = FSBGameplayTags::Get().Attribute_Weight;
			FGameplayTag MaxWeightTag = FSBGameplayTags::Get().Attribute_MaxWeight;

			AttrComp->SetAttributeBaseValue(WeightTag, TotalWeight);

			float MaxWeight = AttrComp->GetAttributeValue(MaxWeightTag);
			if (MaxWeight <= 0.0f)
			{
				MaxWeight = 100.0f;
				AttrComp->SetAttributeBaseValue(MaxWeightTag, MaxWeight);
			}

			USBStateComponent* StateComp = Owner->FindComponentByClass<USBStateComponent>();
			if (StateComp)
			{
				FGameplayTag EncumberedTag = FSBGameplayTags::Get().State_Character_Encumbered;
				if (TotalWeight > MaxWeight)
				{
					if (!StateComp->HasTag(EncumberedTag))
					{
						StateComp->AddTag(EncumberedTag);
					}
				}
				else
				{
					if (StateComp->HasTag(EncumberedTag))
					{
						StateComp->RemoveTag(EncumberedTag);
					}
				}
			}
		}
	}
}

bool USBInventoryComponent::ServerTransferItem(USBInventoryComponent* TargetInventory, USBItemInstance* ItemInstance, int32 Quantity)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !TargetInventory || !ItemInstance || Quantity <= 0)
	{
		return false;
	}

	// 1. Valida se o item existe neste inventário e se há quantidade suficiente
	FSBInventoryEntry* SourceEntry = nullptr;
	for (FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance == ItemInstance)
		{
			SourceEntry = &Entry;
			break;
		}
	}

	if (!SourceEntry || SourceEntry->StackCount < Quantity)
	{
		return false;
	}

	// 2. Valida distância entre os donos se um deles for físico no mundo (não player)
	AActor* SourceOwner = GetOwner();
	AActor* TargetOwner = TargetInventory->GetOwner();
	if (SourceOwner && TargetOwner)
	{
		float Dist = FVector::Dist(SourceOwner->GetActorLocation(), TargetOwner->GetActorLocation());
		if (Dist > 600.0f)
		{
			return false;
		}
	}

	// 3. Adiciona o item ao inventário de destino preservando metadados se for uma nova instância
	USBItemDefinition* ItemDef = const_cast<USBItemDefinition*>(ItemInstance->ItemDef.Get());
	USBItemInstance* TargetInstance = TargetInventory->ServerAddItem(ItemDef, Quantity);
	if (TargetInstance)
	{
		if (TargetInstance->StackCount == Quantity)
		{
			TargetInstance->Durability = ItemInstance->Durability;
			TargetInstance->DynamicTags = ItemInstance->DynamicTags;
			TargetInventory->MarkItemInstanceUpdated(TargetInstance);
		}
	}

	// 4. Remove a quantidade transferida do inventário de origem
	ServerRemoveItem(ItemInstance, Quantity);

	return true;
}

float USBInventoryComponent::CalculateEffectiveDefense(const USBItemInstance* ItemInstance) const
{
	if (!ItemInstance || !ItemInstance->ItemDef)
	{
		return 0.0f;
	}

	const USBItemFragment_Armor* ArmorFragment = Cast<USBItemFragment_Armor>(ItemInstance->FindFragmentByClass(USBItemFragment_Armor::StaticClass()));
	if (!ArmorFragment)
	{
		return 0.0f;
	}

	// Magnitude de defesa base nos modificadores
	float BaseDefense = 0.0f;
	for (const FSBItemAttributeModifier& ModConfig : ArmorFragment->ModifiersToGrant)
	{
		if (ModConfig.AttributeTag == FSBGameplayTags::Get().Attribute_Defense)
		{
			BaseDefense += ModConfig.Magnitude;
		}
	}

	// Aplica multiplicador do upgrade se houver
	float UpgradeMultiplier = 1.0f;
	const USBItemFragment* FoundUpgradeFrag = ItemInstance->FindFragmentByClass(USBItemFragment_Upgrade::StaticClass());
	if (const USBItemFragment_Upgrade* UpgradeFrag = Cast<USBItemFragment_Upgrade>(FoundUpgradeFrag))
	{
		for (int32 i = 0; i < ItemInstance->UpgradeLevel; ++i)
		{
			if (UpgradeFrag->UpgradeLevels.IsValidIndex(i))
			{
				UpgradeMultiplier += UpgradeFrag->UpgradeLevels[i].StatMultiplierBonus;
			}
		}
	}

	return BaseDefense * UpgradeMultiplier;
}

bool USBInventoryComponent::ServerAutoEquipBestArmor(FGameplayTag SlotTag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !SlotTag.IsValid())
	{
		return false;
	}

	USBItemInstance* CurrentEquipped = nullptr;
	FGameplayTag EquippedTag = FSBGameplayTags::Get().State_Item_Equipped;

	// 1. Encontra qual item está atualmente equipado neste slot
	for (const FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance)
		{
			const USBItemFragment_Armor* ArmorFragment = Cast<USBItemFragment_Armor>(Entry.Instance->FindFragmentByClass(USBItemFragment_Armor::StaticClass()));
			if (ArmorFragment && ArmorFragment->EquipmentSlotTag == SlotTag)
			{
				if (EquippedTag.IsValid() && Entry.Instance->DynamicTags.HasTagExact(EquippedTag))
				{
					CurrentEquipped = Entry.Instance;
					break;
				}
			}
		}
	}

	float CurrentDefense = CurrentEquipped ? CalculateEffectiveDefense(CurrentEquipped) : -1.0f;

	USBItemInstance* BestArmor = nullptr;
	float BestDefense = CurrentDefense;

	// 2. Varre o inventário em busca de uma armadura melhor (não equipada) para o mesmo slot
	for (const FSBInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance && Entry.Instance != CurrentEquipped)
		{
			const USBItemFragment_Armor* ArmorFragment = Cast<USBItemFragment_Armor>(Entry.Instance->FindFragmentByClass(USBItemFragment_Armor::StaticClass()));
			if (ArmorFragment && ArmorFragment->EquipmentSlotTag == SlotTag)
			{
				if (!EquippedTag.IsValid() || !Entry.Instance->DynamicTags.HasTagExact(EquippedTag))
				{
					float Def = CalculateEffectiveDefense(Entry.Instance);
					if (Def > BestDefense)
					{
						BestDefense = Def;
						BestArmor = Entry.Instance;
					}
				}
			}
		}
	}

	// 3. Se encontrou um item melhor, faz a troca!
	if (BestArmor)
	{
		if (CurrentEquipped)
		{
			ServerUnequipItem(CurrentEquipped);
		}
		ServerEquipItem(BestArmor);
		return true;
	}

	return false;
}

void USBInventoryComponent::ServerAutoEquipBestArmorAllSlots()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	ServerAutoEquipBestArmor(Tags.Equipment_Slot_Head);
	ServerAutoEquipBestArmor(Tags.Equipment_Slot_Chest);
	ServerAutoEquipBestArmor(Tags.Equipment_Slot_Feet);
	ServerAutoEquipBestArmor(Tags.Equipment_Slot_Hands);
	ServerAutoEquipBestArmor(Tags.Equipment_Slot_Ring);
}

void USBInventoryComponent::DeactivateArmorModifiers(USBItemInstance* ItemInstance)
{
	if (!ItemInstance || !ItemInstance->ItemDef) return;
	const USBItemFragment_Armor* ArmorFragment = Cast<USBItemFragment_Armor>(ItemInstance->FindFragmentByClass(USBItemFragment_Armor::StaticClass()));
	if (!ArmorFragment) return;

	AActor* Owner = GetOwner();
	USBAttributeComponent* AttrComp = Owner ? Owner->FindComponentByClass<USBAttributeComponent>() : nullptr;
	if (AttrComp)
	{
		FGameplayTag SourceTag = ArmorFragment->EquipmentSlotTag.IsValid() ? ArmorFragment->EquipmentSlotTag : FSBGameplayTags::Get().State_Item_Equipped;
		for (const FSBItemAttributeModifier& ModConfig : ArmorFragment->ModifiersToGrant)
		{
			if (ModConfig.AttributeTag.IsValid())
			{
				AttrComp->RemoveModifiersBySource(ModConfig.AttributeTag, SourceTag);
			}
		}
	}
}

void USBInventoryComponent::HandleOwnerAttributeChanged(FGameplayTag AttributeTag, float NewValue, float OldValue, AActor* Instigator)
{
	if (AttributeTag == FSBGameplayTags::Get().Attribute_Health && NewValue < OldValue)
	{
		float DamageTaken = OldValue - NewValue;
		if (DamageTaken > 0.0f)
		{
			TArray<USBItemInstance*> EquippedArmors;
			FGameplayTag EquippedTag = FSBGameplayTags::Get().State_Item_Equipped;

			for (const FSBInventoryEntry& Entry : InventoryList.Entries)
			{
				if (Entry.Instance && Entry.Instance->DynamicTags.HasTagExact(EquippedTag))
				{
					const USBItemFragment_Armor* ArmorFrag = Cast<USBItemFragment_Armor>(Entry.Instance->FindFragmentByClass(USBItemFragment_Armor::StaticClass()));
					if (ArmorFrag)
					{
						EquippedArmors.Add(Entry.Instance);
					}
				}
			}

			if (EquippedArmors.Num() > 0)
			{
				// Reduz durabilidade de cada armadura equipada proporcional ao dano
				for (USBItemInstance* ArmorInstance : EquippedArmors)
				{
					float CurrentDur = ArmorInstance->Durability;
					if (CurrentDur > 0.0f)
					{
						float NewDur = FMath::Max(0.0f, CurrentDur - DamageTaken);
						ArmorInstance->SetDurability_Implementation(NewDur);
						MarkItemInstanceUpdated(ArmorInstance);

						if (NewDur <= 0.0f)
						{
							DeactivateArmorModifiers(ArmorInstance);
						}
					}
				}
			}
		}
	}
}
