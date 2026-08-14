#include "SandboxInventory.h"
#include "GameplayTagsManager.h"

#define LOCTEXT_NAMESPACE "FSandboxInventoryModule"

void FSandboxInventoryModule::StartupModule()
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.ItemEquipped"), TEXT("Fired when an item is equipped"));
	TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.ItemUnequipped"), TEXT("Fired when an item is unequipped"));
	TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.SlotUpdated"), TEXT("Fired when an inventory slot's item or quantity changes"));
	TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.ItemAdded"), TEXT("Fired when an item is added to the inventory"));
	TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.ItemRemoved"), TEXT("Fired when an item is removed from the inventory"));
}

void FSandboxInventoryModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSandboxInventoryModule, SandboxInventory)
