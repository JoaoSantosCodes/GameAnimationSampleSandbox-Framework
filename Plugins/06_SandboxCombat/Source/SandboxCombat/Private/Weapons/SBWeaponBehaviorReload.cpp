#include "Weapons/SBWeaponBehaviorReload.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"
#include "Components/SBBehaviorStackComponent.h"

USBWeaponBehaviorReload::USBWeaponBehaviorReload()
{
}

bool USBWeaponBehaviorReload::CanEnter_Implementation(const FSBBehaviorContext& Context) const
{
	if (!Super::CanEnter_Implementation(Context))
	{
		return false;
	}

	USBAttributeComponent* AttrComp = Cast<USBAttributeComponent>(CachedAttributeComponent);
	USBStateComponent* StateComp = Cast<USBStateComponent>(CachedStateComponent);
	if (!AttrComp || !StateComp)
	{
		return false;
	}

	// Impedir recarga se já estiver recarregando
	FGameplayTag ReloadingTag = FSBGameplayTags::Get().State_Character_Reloading;
	if (ReloadingTag.IsValid() && StateComp->HasTag(ReloadingTag))
	{
		return false;
	}

	// Impedir recarga se a munição já estiver cheia
	FGameplayTag AmmoTag = FSBGameplayTags::Get().Attribute_Weapon_Ammo;
	FSBAttribute AmmoAttr;
	if (AmmoTag.IsValid() && AttrComp->GetAttribute(AmmoTag, AmmoAttr))
	{
		if (AmmoAttr.BaseValue >= AmmoAttr.MaxValue)
		{
			return false;
		}
	}

	return true;
}

void USBWeaponBehaviorReload::Enter_Implementation(const FSBBehaviorContext& Context)
{
	Super::Enter_Implementation(Context);

	USBStateComponent* StateComp = Cast<USBStateComponent>(CachedStateComponent);
	if (StateComp)
	{
		FGameplayTag ReloadingTag = FSBGameplayTags::Get().State_Character_Reloading;
		if (ReloadingTag.IsValid())
		{
			StateComp->AddTag(ReloadingTag);
		}
	}

	ReloadTimeElapsed = 0.0f;
}

void USBWeaponBehaviorReload::Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context)
{
	Super::Update_Implementation(DeltaTime, Context);

	ReloadTimeElapsed += DeltaTime;

	if (ReloadTimeElapsed >= ReloadDuration)
	{
		USBAttributeComponent* AttrComp = Cast<USBAttributeComponent>(CachedAttributeComponent);
		if (AttrComp)
		{
			FGameplayTag AmmoTag = FSBGameplayTags::Get().Attribute_Weapon_Ammo;
			FSBAttribute AmmoAttr;
			if (AmmoTag.IsValid() && AttrComp->GetAttribute(AmmoTag, AmmoAttr))
			{
				AttrComp->SetAttributeBaseValue(AmmoTag, AmmoAttr.MaxValue);
			}
		}

		if (StackComponent)
		{
			StackComponent->StopBehavior(GetBehaviorTag());
		}
	}
}

void USBWeaponBehaviorReload::Exit_Implementation(const FSBBehaviorContext& Context)
{
	USBStateComponent* StateComp = Cast<USBStateComponent>(CachedStateComponent);
	if (StateComp)
	{
		FGameplayTag ReloadingTag = FSBGameplayTags::Get().State_Character_Reloading;
		if (ReloadingTag.IsValid())
		{
			StateComp->RemoveTag(ReloadingTag);
		}
	}

	Super::Exit_Implementation(Context);
}
