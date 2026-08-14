#include "Weapons/SBWeaponBehavior.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "GameFramework/Actor.h"

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

	if (CombatStateComponent)
	{
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
			FGameplayTag AmmoTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Weapon.Ammo"));
			if (CombatAttributeComponent->GetAttributeValue(AmmoTag) < WeaponDefinition->AmmoCost)
			{
				return false;
			}
		}

		if (WeaponDefinition->ManaCost > 0.0f)
		{
			FGameplayTag ManaTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Mana"));
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
