#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class SANDBOXCOMMON_API FSBGameplayTags
{
public:
	static const FSBGameplayTags& Get() { return Instance; }
	static void InitializeNativeTags();

	// Character States
	FGameplayTag State_Character_Idle;
	FGameplayTag State_Character_Walking;
	FGameplayTag State_Character_Sprinting;
	FGameplayTag State_Character_Falling;
	FGameplayTag State_Character_Aiming;
	FGameplayTag State_Character_Reloading;
	FGameplayTag State_Character_Interacting;
	FGameplayTag State_Character_Dead;
	FGameplayTag State_Character_Stunned;
	FGameplayTag State_Character_Crouching;
	FGameplayTag State_Character_Exhausted;
	FGameplayTag State_Weapon_Firing;

	// Inputs
	FGameplayTag Input_Action_Move;
	FGameplayTag Input_Action_Look;
	FGameplayTag Input_Action_Jump;
	FGameplayTag Input_Action_Sprint;
	FGameplayTag Input_Action_Fire;
	FGameplayTag Input_Action_Reload;
	FGameplayTag Input_Action_Interact;

	// Events
	FGameplayTag Event_Character_Damaged;
	FGameplayTag Event_Character_Dead;
	FGameplayTag Event_Weapon_Fire;
	FGameplayTag Combat_Action_Reload;

	// Attributes
	FGameplayTag Attribute_Health;
	FGameplayTag Attribute_Stamina;
	FGameplayTag Attribute_Mana;
	FGameplayTag Attribute_Weapon_Ammo;

private:
	static FSBGameplayTags Instance;
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);
};
