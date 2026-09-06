#include "Weapons/SBWeaponBehavior.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "GameFramework/Actor.h"
#include "SBGameplayTags.h"
#include "Interfaces/SBItemDurabilityInterface.h"

USBWeaponBehavior::USBWeaponBehavior()
{
}

void USBWeaponBehavior::Initialize(USBBehaviorStackComponent* InStackComp, USBGameplayBehaviorDefinition* InDefinition)
{
	Super::Initialize(InStackComp, InDefinition);
	CombatComponent = Cast<USBCombatComponent>(InStackComp);
	WeaponDefinition = Cast<USBWeaponBehaviorDefinition>(InDefinition);

	CombatAttributeComponent = Cast<USBAttributeComponent>(USBGameplayBehavior::CachedAttributeComponent);
	CombatStateComponent = Cast<USBStateComponent>(USBGameplayBehavior::CachedStateComponent);
}

bool USBWeaponBehavior::CanEnter_Implementation(const FSBBehaviorContext& Context) const
{
	if (!WeaponDefinition)
	{
		return false;
	}

	if (EquippedItemInstance.IsValid() && EquippedItemInstance->GetClass()->ImplementsInterface(USBItemDurabilityInterface::StaticClass()))
	{
		float CurrentDurability = ISBItemDurabilityInterface::Execute_GetDurability(EquippedItemInstance.Get());
		if (CurrentDurability <= 0.0f)
		{
			return false;
		}
	}

	if (CombatStateComponent)
	{
		FGameplayTag ReloadingTag = FSBGameplayTags::Get().State_Character_Reloading;
		if (ReloadingTag.IsValid() && CombatStateComponent->HasTag(ReloadingTag))
		{
			return false;
		}

		if (!CombatStateComponent->HasAll(WeaponDefinition->RequiredTags))
		{
			return false;
		}

		if (CombatStateComponent->HasAny(WeaponDefinition->BlockedTags))
		{
			return false;
		}
	}

	if (CombatAttributeComponent)
	{
		if (WeaponDefinition->AmmoCost > 0.0f)
		{
			FGameplayTag AmmoTag = FSBGameplayTags::Get().Attribute_Weapon_Ammo;
			if (CombatAttributeComponent->GetAttributeValue(AmmoTag) < WeaponDefinition->AmmoCost)
			{
				return false;
			}
		}

		if (WeaponDefinition->ManaCost > 0.0f)
		{
			FGameplayTag ManaTag = FSBGameplayTags::Get().Attribute_Mana;
			if (CombatAttributeComponent->GetAttributeValue(ManaTag) < WeaponDefinition->ManaCost)
			{
				return false;
			}
		}
	}

	return true;
}

bool USBWeaponBehavior::CanExit_Implementation(const FSBBehaviorContext& Context) const
{
	return true;
}

void USBWeaponBehavior::Enter_Implementation(const FSBBehaviorContext& Context)
{
	if (CombatComponent)
	{
		CombatComponent->SetWeaponVisualActive(GetBehaviorTag(), true);
	}

	// Consome durabilidade autoritativamente no servidor
	if (CombatComponent && CombatComponent->GetOwner() && CombatComponent->GetOwner()->HasAuthority() &&
		EquippedItemInstance.IsValid() && WeaponDefinition.Get() && WeaponDefinition->DurabilityCost > 0.0f)
	{
		if (EquippedItemInstance->GetClass()->ImplementsInterface(USBItemDurabilityInterface::StaticClass()))
		{
			ISBItemDurabilityInterface::Execute_ConsumeDurability(EquippedItemInstance.Get(), WeaponDefinition->DurabilityCost);

			// Notifica o componente de inventário via reflexão para marcar a replicação da durabilidade
			AActor* Owner = CombatComponent->GetOwner();
			if (Owner)
			{
				UActorComponent* InvComp = Owner->GetComponentByClass(FindObject<UClass>(nullptr, TEXT("/Script/SandboxInventory.SBInventoryComponent")));
				if (InvComp)
				{
					UFunction* MarkUpdatedFunc = InvComp->GetClass()->FindFunctionByName(TEXT("MarkItemInstanceUpdated"));
					if (MarkUpdatedFunc)
					{
						struct FMarkUpdatedParams
						{
							UObject* ItemInstance;
						};
						FMarkUpdatedParams Params;
						Params.ItemInstance = EquippedItemInstance.Get();
						InvComp->ProcessEvent(MarkUpdatedFunc, &Params);
					}
				}
			}
		}
	}
}

void USBWeaponBehavior::Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context)
{
}

void USBWeaponBehavior::Exit_Implementation(const FSBBehaviorContext& Context)
{
	if (CombatComponent)
	{
		CombatComponent->SetWeaponVisualActive(GetBehaviorTag(), false);
	}
}

int32 USBWeaponBehavior::GetStackPriority() const
{
	return WeaponDefinition ? WeaponDefinition->StackPriority : 0;
}
