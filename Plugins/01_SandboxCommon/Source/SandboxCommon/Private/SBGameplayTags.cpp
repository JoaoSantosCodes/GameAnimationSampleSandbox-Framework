#include "SBGameplayTags.h"
#include "GameplayTagsManager.h"

FSBGameplayTags FSBGameplayTags::Instance;

void FSBGameplayTags::InitializeNativeTags()
{
	Instance.AddTag(Instance.State_Character_Idle, "State.Character.Idle", "Character is doing nothing");
	Instance.AddTag(Instance.State_Character_Walking, "State.Character.Walking", "Character is walking");
	Instance.AddTag(Instance.State_Character_Sprinting, "State.Character.Sprinting", "Character is sprinting");
	Instance.AddTag(Instance.State_Character_Falling, "State.Character.Falling", "Character is in mid-air");
	Instance.AddTag(Instance.State_Character_Aiming, "State.Character.Aiming", "Character is aiming a weapon");
	Instance.AddTag(Instance.State_Character_Reloading, "State.Character.Reloading", "Character is reloading a weapon");
	Instance.AddTag(Instance.State_Character_Interacting, "State.Character.Interacting", "Character is interacting with an object");
	Instance.AddTag(Instance.State_Character_Dead, "State.Character.Dead", "Character is dead");
	Instance.AddTag(Instance.State_Character_Stunned, "State.Character.Stunned", "Character is stunned");
	Instance.AddTag(Instance.State_Character_Crouching, "State.Character.Crouching", "Character is crouching");
	Instance.AddTag(Instance.State_Weapon_Firing, "State.Weapon.Firing", "Weapon is firing");

	Instance.AddTag(Instance.Input_Action_Move, "Input.Action.Move", "Movement input action");
	Instance.AddTag(Instance.Input_Action_Look, "Input.Action.Look", "Look input action");
	Instance.AddTag(Instance.Input_Action_Jump, "Input.Action.Jump", "Jump input action");
	Instance.AddTag(Instance.Input_Action_Sprint, "Input.Action.Sprint", "Sprint input action");
	Instance.AddTag(Instance.Input_Action_Fire, "Input.Action.Fire", "Fire input action");
	Instance.AddTag(Instance.Input_Action_Reload, "Input.Action.Reload", "Reload input action");
	Instance.AddTag(Instance.Input_Action_Interact, "Input.Action.Interact", "Interact input action");

	Instance.AddTag(Instance.Event_Character_Damaged, "Event.Character.Damaged", "Fired when character takes damage");
	Instance.AddTag(Instance.Event_Character_Dead, "Event.Character.Dead", "Fired when character dies");
	Instance.AddTag(Instance.Event_Weapon_Fire, "Event.Weapon.Fire", "Fired when weapon is shot");

	Instance.AddTag(Instance.Attribute_Health, "Attribute.Health", "Health attribute tag");
	Instance.AddTag(Instance.Attribute_Stamina, "Attribute.Stamina", "Stamina attribute tag");
	Instance.AddTag(Instance.Attribute_Mana, "Attribute.Mana", "Mana attribute tag");
	Instance.AddTag(Instance.Attribute_Weapon_Ammo, "Attribute.Weapon.Ammo", "Weapon Ammo attribute tag");
}

void FSBGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment)
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	OutTag = TagsManager.AddNativeGameplayTag(FName(TagName), FString(TagComment));
}
