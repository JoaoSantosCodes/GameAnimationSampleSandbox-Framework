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
	Instance.AddTag(Instance.State_Character_Frozen, "State.Character.Frozen", "Character is frozen");
	Instance.AddTag(Instance.State_Character_Crouching, "State.Character.Crouching", "Character is crouching");
	Instance.AddTag(Instance.State_Character_Exhausted, "State.Character.Exhausted", "Character is exhausted from stamina loss");
	Instance.AddTag(Instance.State_Character_HitReacting, "State.Character.HitReacting", "Character is reacting to hit damage");
	Instance.AddTag(Instance.State_Character_Encumbered, "State.Character.Encumbered", "Character is encumbered by carrying too much weight");
	Instance.AddTag(Instance.State_Weapon_Firing, "State.Weapon.Firing", "Weapon is firing");
	Instance.AddTag(Instance.State_Item_Equipped, "State.Item.Equipped", "Item is equipped");

	Instance.AddTag(Instance.Movement_Action_Sprint, "Movement.Action.Sprint", "Sprint movement action");

	Instance.AddTag(Instance.Input_Action_Move, "Input.Action.Move", "Movement input action");
	Instance.AddTag(Instance.Input_Action_Look, "Input.Action.Look", "Look input action");
	Instance.AddTag(Instance.Input_Action_Jump, "Input.Action.Jump", "Jump input action");
	Instance.AddTag(Instance.Input_Action_Sprint, "Input.Action.Sprint", "Sprint input action");
	Instance.AddTag(Instance.Input_Action_Fire, "Input.Action.Fire", "Fire input action");
	Instance.AddTag(Instance.Input_Action_Reload, "Input.Action.Reload", "Reload input action");
	Instance.AddTag(Instance.Input_Action_Interact, "Input.Action.Interact", "Interact input action");

	Instance.AddTag(Instance.Event_Character_Damaged, "Event.Character.Damaged", "Fired when character takes damage");
	Instance.AddTag(Instance.Event_Character_Dead, "Event.Character.Dead", "Fired when character dies");
	Instance.AddTag(Instance.Event_Character_Footstep, "Event.Character.Footstep", "Fired when character takes a physical footstep");
	Instance.AddTag(Instance.Event_Weapon_Fire, "Event.Weapon.Fire", "Fired when weapon is shot");
	Instance.AddTag(Instance.Event_Combat_HitReact, "Event.Combat.HitReact", "Fired when a character suffers a combat hit");
	Instance.AddTag(Instance.Event_Combat_CriticalHit, "Event.Combat.CriticalHit", "Fired when a critical hit / headshot is scored");
	Instance.AddTag(Instance.Combat_Action_Reload, "Combat.Action.Reload", "Reload weapon action");
	Instance.AddTag(Instance.Combat_Action_Fire, "Combat.Action.Fire", "Fire weapon action");

	Instance.AddTag(Instance.Event_Attribute_Changed, "Event.Attribute.Changed", "Fired when an attribute changes value");

	Instance.AddTag(Instance.Event_Interaction_Available, "Event.Interaction.Available", "Fired when an interaction becomes available");
	Instance.AddTag(Instance.Event_Interaction_Cleared, "Event.Interaction.Cleared", "Fired when an interaction is cleared");
	Instance.AddTag(Instance.Event_Interaction_Progress, "Event.Interaction.Progress", "Fired during interaction progress");
	Instance.AddTag(Instance.Event_Interaction_Started, "Event.Interaction.Started", "Fired when an interaction starts");
	Instance.AddTag(Instance.Event_Interaction_Completed, "Event.Interaction.Completed", "Fired when an interaction completes");

	Instance.AddTag(Instance.Event_Area_Discovered, "Event.Area.Discovered", "Fired when a new area is entered/discovered");

	Instance.AddTag(Instance.Feature_Combat, "Feature.Combat", "Combat gameplay feature flag");
	Instance.AddTag(Instance.Feature_Inventory, "Feature.Inventory", "Inventory gameplay feature flag");
	Instance.AddTag(Instance.Feature_Crafting, "Feature.Crafting", "Crafting gameplay feature flag");
	Instance.AddTag(Instance.Feature_Building, "Feature.Building", "Building gameplay feature flag");
	Instance.AddTag(Instance.Feature_Quests, "Feature.Quests", "Quests gameplay feature flag");
	Instance.AddTag(Instance.Event_Feature_Toggled, "Event.Feature.Toggled", "Fired when a feature state is toggled");

	Instance.AddTag(Instance.SimType_ResourceNode, "SimType.ResourceNode", "Resource node background simulation type");
	Instance.AddTag(Instance.State_Timer_Respawn, "State.Timer.Respawn", "Respawn timer state for background simulation");

	Instance.AddTag(Instance.Event_Inventory_ItemAdded, "Event.Inventory.ItemAdded", "Fired when an item is added to inventory");
	Instance.AddTag(Instance.Event_Inventory_ItemRemoved, "Event.Inventory.ItemRemoved", "Fired when an item is removed from inventory");
	Instance.AddTag(Instance.Event_Inventory_ItemEquipped, "Event.Inventory.ItemEquipped", "Fired when an item is equipped");
	Instance.AddTag(Instance.Event_Inventory_ItemUnequipped, "Event.Inventory.ItemUnequipped", "Fired when an item is unequipped");
	Instance.AddTag(Instance.Event_Inventory_SlotUpdated, "Event.Inventory.SlotUpdated", "Fired when an inventory slot updates");

	Instance.AddTag(Instance.Event_Ability_CooldownStarted, "Event.Ability.CooldownStarted", "Fired when an ability cooldown starts");
	Instance.AddTag(Instance.Event_Ability_CooldownEnded, "Event.Ability.CooldownEnded", "Fired when an ability cooldown ends");

	Instance.AddTag(Instance.Event_Quest_RewardsClaimed, "Event.Quest.RewardsClaimed", "Fired when a quest's rewards are successfully claimed by a player");

	Instance.AddTag(Instance.Attribute_Health, "Attribute.Health", "Health attribute tag");
	Instance.AddTag(Instance.Attribute_MaxHealth, "Attribute.MaxHealth", "Maximum health capacity");
	Instance.AddTag(Instance.Attribute_Stamina, "Attribute.Stamina", "Stamina attribute tag");
	Instance.AddTag(Instance.Attribute_Mana, "Attribute.Mana", "Mana attribute tag");
	Instance.AddTag(Instance.Attribute_Speed, "Attribute.Speed", "Speed attribute tag");
	Instance.AddTag(Instance.Attribute_Defense, "Attribute.Defense", "Defense attribute tag");
	Instance.AddTag(Instance.Attribute_Weapon_Ammo, "Attribute.Weapon.Ammo", "Weapon Ammo attribute tag");
	Instance.AddTag(Instance.Attribute_Coins, "Attribute.Coins", "Coins currency attribute tag");
	Instance.AddTag(Instance.Attribute_Weight, "Attribute.Weight", "Current total inventory weight attribute tag");
	Instance.AddTag(Instance.Attribute_MaxWeight, "Attribute.MaxWeight", "Maximum allowed inventory weight attribute tag");

	Instance.AddTag(Instance.Equipment_Slot_Head, "Equipment.Slot.Head", "Head armor equipment slot");
	Instance.AddTag(Instance.Equipment_Slot_Chest, "Equipment.Slot.Chest", "Chest armor equipment slot");
	Instance.AddTag(Instance.Equipment_Slot_Feet, "Equipment.Slot.Feet", "Feet armor equipment slot");
	Instance.AddTag(Instance.Equipment_Slot_Hands, "Equipment.Slot.Hands", "Hands armor equipment slot");
	Instance.AddTag(Instance.Equipment_Slot_Ring, "Equipment.Slot.Ring", "Ring accessory equipment slot");

	Instance.AddTag(Instance.Crafting_Station_Forge, "Crafting.Station.Forge", "Forge crafting station tag");
	Instance.AddTag(Instance.Crafting_Station_Alchemy, "Crafting.Station.Alchemy", "Alchemy crafting station tag");
	Instance.AddTag(Instance.Event_Crafting_Completed, "Event.Crafting.Completed", "Fired when crafting completes successfully");
	Instance.AddTag(Instance.Event_Crafting_Failed, "Event.Crafting.Failed", "Fired when crafting fails");

	Instance.AddTag(Instance.Item_Type_Material, "Item.Type.Material", "Material item type classification");
	Instance.AddTag(Instance.Quest_Objective_Footstep, "Quest.Objective.Footstep", "Quest objective based on character footsteps");

	Instance.AddTag(Instance.Loot_Rarity_Common, "Loot.Rarity.Common", "Common loot rarity");
	Instance.AddTag(Instance.Loot_Rarity_Uncommon, "Loot.Rarity.Uncommon", "Uncommon loot rarity");
	Instance.AddTag(Instance.Loot_Rarity_Rare, "Loot.Rarity.Rare", "Rare loot rarity");
	Instance.AddTag(Instance.Loot_Rarity_Epic, "Loot.Rarity.Epic", "Epic loot rarity");
	Instance.AddTag(Instance.Loot_Rarity_Legendary, "Loot.Rarity.Legendary", "Legendary loot rarity");

	// Region and Zone Types
	Instance.AddTag(Instance.Zone_Type_SafeZone, "Zone.Type.SafeZone", "Designates a sanctuary or safe zone where combat/harm is disabled");
	Instance.AddTag(Instance.Zone_Type_PvP, "Zone.Type.PvP", "Designates a competitive area where PvP is allowed");
	Instance.AddTag(Instance.Zone_Type_Dungeon, "Zone.Type.Dungeon", "Designates an instanced or underground dungeon area");
	Instance.AddTag(Instance.Zone_Type_Hazard, "Zone.Type.Hazard", "Designates a hazardous environmental danger zone");

	// Zone State Tags
	Instance.AddTag(Instance.State_Zone_Safe, "State.Zone.Safe", "Applied to actors currently inside a safe zone");
	Instance.AddTag(Instance.State_Zone_PvPAllowed, "State.Zone.PvPAllowed", "Applied to actors currently inside a PvP zone");
	Instance.AddTag(Instance.State_Zone_InHazard, "State.Zone.InHazard", "Applied to actors currently exposed to environmental hazards");

	// Weather State Tags
	Instance.AddTag(Instance.State_Weather_Clear, "State.Weather.Clear", "Clear sky and standard weather conditions");
	Instance.AddTag(Instance.State_Weather_Rain, "State.Weather.Rain", "Rainfall precipitation weather state");
	Instance.AddTag(Instance.State_Weather_Snow, "State.Weather.Snow", "Snowfall precipitation weather state");
	Instance.AddTag(Instance.State_Weather_Storm, "State.Weather.Storm", "Severe storm with lightning and heavy rain");
	Instance.AddTag(Instance.State_Weather_Fog, "State.Weather.Fog", "Dense fog reducing environmental visibility");

	// Time Period State Tags
	Instance.AddTag(Instance.State_Time_Dawn, "State.Time.Dawn", "Dawn period of early morning (05:00 - 07:00)");
	Instance.AddTag(Instance.State_Time_Day, "State.Time.Day", "Full daylight period (07:00 - 18:00)");
	Instance.AddTag(Instance.State_Time_Dusk, "State.Time.Dusk", "Dusk period of sunset (18:00 - 20:00)");
	Instance.AddTag(Instance.State_Time_Night, "State.Time.Night", "Nighttime darkness period (20:00 - 05:00)");

	// Portal Types
	Instance.AddTag(Instance.Portal_Type_Gateway, "Portal.Type.Gateway", "Two-way planar gateway portal");
	Instance.AddTag(Instance.Portal_Type_Waystone, "Portal.Type.Waystone", "Fast-travel waystone shrine");
	Instance.AddTag(Instance.Portal_Type_DungeonGate, "Portal.Type.DungeonGate", "Entrance gateway to instanced dungeon");

	// Portal State Tags
	Instance.AddTag(Instance.State_Portal_Teleporting, "State.Portal.Teleporting", "Applied to actor actively channeling or undergoing teleportation");
	Instance.AddTag(Instance.State_Portal_Locked, "State.Portal.Locked", "Applied to portal when sealed or locked by key/quest");
	Instance.AddTag(Instance.State_Portal_Cooldown, "State.Portal.Cooldown", "Applied to portal or actor during teleport cooldown");

	// Gameplay Effect Tags
	Instance.AddTag(Instance.Effect_Buff_Berserk, "Effect.Buff.Berserk", "Berserk state granting bonus damage but reduced defense");
	Instance.AddTag(Instance.Effect_Buff_SpeedBoost, "Effect.Buff.SpeedBoost", "Increases movement speed by percentage");
	Instance.AddTag(Instance.Effect_Buff_Regeneration, "Effect.Buff.Regeneration", "Periodic healing over time (HoT)");
	Instance.AddTag(Instance.Effect_Debuff_Poison, "Effect.Debuff.Poison", "Periodic toxic damage over time (DoT)");
	Instance.AddTag(Instance.Effect_Debuff_Burn, "Effect.Debuff.Burn", "Periodic fire damage over time (DoT)");
	Instance.AddTag(Instance.Effect_Debuff_Slow, "Effect.Debuff.Slow", "Reduces movement speed");
	Instance.AddTag(Instance.Effect_Debuff_Stun, "Effect.Debuff.Stun", "Incapacitates character action execution");

	// Immunity State Tags
	Instance.AddTag(Instance.State_Immunity_Poison, "State.Immunity.Poison", "Grants complete immunity against poison effects");
	Instance.AddTag(Instance.State_Immunity_Stun, "State.Immunity.Stun", "Grants complete immunity against stun crowd control");
	Instance.AddTag(Instance.State_Immunity_Burn, "State.Immunity.Burn", "Grants complete immunity against burning effects");

	// Combo Tags
	Instance.AddTag(Instance.Combat_Combo_Light, "Combat.Combo.Light", "Light attack combo sequence node");
	Instance.AddTag(Instance.Combat_Combo_Heavy, "Combat.Combo.Heavy", "Heavy attack combo sequence node");
	Instance.AddTag(Instance.Combat_Combo_Finisher, "Combat.Combo.Finisher", "High-impact final combo attack");

	// Combo State Tags
	Instance.AddTag(Instance.State_Combat_ComboWindowOpen, "State.Combat.ComboWindowOpen", "Indicates the timing window to chain the next combo attack is active");
	Instance.AddTag(Instance.State_Combat_FinisherReady, "State.Combat.FinisherReady", "Indicates that the combo sequence reached max chain and finisher is primed");

	// Defense & Parry Tags
	Instance.AddTag(Instance.State_Combat_Blocking, "State.Combat.Blocking", "Actor is actively holding a defensive guard / shield block");
	Instance.AddTag(Instance.State_Combat_ParryWindow, "State.Combat.ParryWindow", "Actor is in the initial frames of block capable of perfect parry deflection");
	Instance.AddTag(Instance.State_Combat_GuardBroken, "State.Combat.GuardBroken", "Actor guard broken due to stamina depletion during incoming hit");
	Instance.AddTag(Instance.State_Combat_CounterAttackReady, "State.Combat.CounterAttackReady", "Actor successfully executed a parry and has an empowered counter-attack window");
	Instance.AddTag(Instance.State_Combat_Staggered, "State.Combat.Staggered", "Attacker or target is staggered / reel back from a deflected strike");

	// Lock-On Tags
	Instance.AddTag(Instance.State_Combat_LockedOn, "State.Combat.LockedOn", "Attacking actor has an active lock-on target");
	Instance.AddTag(Instance.State_Combat_Target, "State.Combat.Target", "Target actor is currently locked-on by a player or AI");

	// Hit Trace & Melee Attack Tags
	Instance.AddTag(Instance.State_Combat_Attacking, "State.Combat.Attacking", "Actor is actively executing an attack action");
	Instance.AddTag(Instance.State_Combat_HitTraceActive, "State.Combat.HitTraceActive", "Melee weapon collision traces are active during attack swing window");

	// Poise & Hit Reaction Tags
	Instance.AddTag(Instance.State_Combat_SuperArmor, "State.Combat.SuperArmor", "Actor ignores hit flinching and action interruption");
	Instance.AddTag(Instance.State_Combat_PoiseBroken, "State.Combat.PoiseBroken", "Actor poise health depleted resulting in posture break / stagger");
	Instance.AddTag(Instance.Combat_Reaction_Front, "Combat.Reaction.Front", "Directional hit reaction from frontal strike");
	Instance.AddTag(Instance.Combat_Reaction_Back, "Combat.Reaction.Back", "Directional hit reaction from back strike");
	Instance.AddTag(Instance.Combat_Reaction_Left, "Combat.Reaction.Left", "Directional hit reaction from left flank strike");
	Instance.AddTag(Instance.Combat_Reaction_Right, "Combat.Reaction.Right", "Directional hit reaction from right flank strike");

	// Motion Warping Tags
	Instance.AddTag(Instance.State_Combat_MotionWarping, "State.Combat.MotionWarping", "Actor is actively being translated and rotated toward a target during an attack animation");

	// Execution & Finisher Tags
	Instance.AddTag(Instance.State_Combat_Executing, "State.Combat.Executing", "Attacker is currently executing a paired finisher sequence");
	Instance.AddTag(Instance.State_Combat_Executed, "State.Combat.Executed", "Victim is locked in a paired execution animation");
	Instance.AddTag(Instance.State_Combat_Invulnerable, "State.Combat.Invulnerable", "Actor is immune to external incoming damage");

	// Combat FX Tags
	Instance.AddTag(Instance.State_Combat_WeaponTrailActive, "State.Combat.WeaponTrailActive", "Melee weapon ribbon/trail visual effect is actively emitting");

	// Combat Feedback Tags
	Instance.AddTag(Instance.State_Combat_HitStop, "State.Combat.HitStop", "Actor is experiencing hit-stop micro freeze frame upon impact");
	Instance.AddTag(Instance.State_Combat_Slomo, "State.Combat.Slomo", "Actor or scene is in cinematic temporal dilation");

	// Dismemberment Tags
	Instance.AddTag(Instance.State_Combat_Dismembered, "State.Combat.Dismembered", "Actor has suffered skeletal limb amputation/dismemberment");
	Instance.AddTag(Instance.Combat_Dismember_Head, "Combat.Dismember.Head", "Decapitation / head limb severed");
	Instance.AddTag(Instance.Combat_Dismember_Arm, "Combat.Dismember.Arm", "Arm limb severed");
	Instance.AddTag(Instance.Combat_Dismember_Leg, "Combat.Dismember.Leg", "Leg limb severed");

	// Stealth & Perception Tags
	Instance.AddTag(Instance.State_Combat_Stealth_Hidden, "State.Combat.Stealth.Hidden", "Actor is completely hidden from enemy perception");
	Instance.AddTag(Instance.State_Combat_Stealth_Suspicious, "State.Combat.Stealth.Suspicious", "Enemies are actively investigating or accumulating alert on actor");
	Instance.AddTag(Instance.State_Combat_Stealth_Detected, "State.Combat.Stealth.Detected", "Actor is fully detected and in combat state");

	// Cover System Tags
	Instance.AddTag(Instance.State_Combat_InCover, "State.Combat.InCover", "Actor is positioned and anchored behind cover");
	Instance.AddTag(Instance.State_Combat_InCover_Low, "State.Combat.InCover.Low", "Actor is in low/crouched cover stance");
	Instance.AddTag(Instance.State_Combat_InCover_High, "State.Combat.InCover.High", "Actor is in high/standing cover stance");
	Instance.AddTag(Instance.State_Combat_Peeking, "State.Combat.Peeking", "Actor is actively leaning or peeking from behind cover");

	// Parkour & Locomotion Tags
	Instance.AddTag(Instance.State_Movement_ParkourActive, "State.Movement.ParkourActive", "Actor is actively executing a parkour or vaulting action");
	Instance.AddTag(Instance.State_Movement_Vaulting, "State.Movement.Vaulting", "Actor is vaulting over a low obstacle");
	Instance.AddTag(Instance.State_Movement_Mantling, "State.Movement.Mantling", "Actor is mantling or climbing up onto a high ledge");

	// Foot IK & Grounding Tags
	Instance.AddTag(Instance.State_Movement_FootIKActive, "State.Movement.FootIKActive", "Foot IK alignment and grounding system is actively compensating terrain");
	Instance.AddTag(Instance.State_Movement_OnSlope, "State.Movement.OnSlope", "Actor is standing or moving on an inclined slope or ramp");

	// Mount & Riding Tags
	Instance.AddTag(Instance.State_Movement_Mounted, "State.Movement.Mounted", "Actor is riding or mounted onto a creature or vehicle");
	Instance.AddTag(Instance.State_Movement_Mounting, "State.Movement.Mounting", "Actor is actively transitioning/mounting onto a mount");
	Instance.AddTag(Instance.State_Movement_Dismounting, "State.Movement.Dismounting", "Actor is actively dismounting from a mount");
	Instance.AddTag(Instance.State_Movement_Galloping, "State.Movement.Galloping", "Mount is actively sprinting or galloping at top speed");

	// Swimming & Water Locomotion Tags
	Instance.AddTag(Instance.State_Movement_Swimming, "State.Movement.Swimming", "Actor is in a body of water and swimming");
	Instance.AddTag(Instance.State_Movement_Swimming_Surface, "State.Movement.Swimming.Surface", "Actor is swimming at the surface of the water");
	Instance.AddTag(Instance.State_Movement_Swimming_Diving, "State.Movement.Swimming.Diving", "Actor is diving underwater submerged");
	Instance.AddTag(Instance.State_Status_Drowning, "State.Status.Drowning", "Actor has depleted oxygen while submerged and is taking drowning damage");

	// Gliding & Aerial Locomotion Tags
	Instance.AddTag(Instance.State_Movement_Gliding, "State.Movement.Gliding", "Actor is actively gliding or parachuting in mid-air");
	Instance.AddTag(Instance.State_Movement_Gliding_Deploying, "State.Movement.Gliding.Deploying", "Actor is actively deploying glider canopy");
	Instance.AddTag(Instance.State_Movement_Gliding_Diving, "State.Movement.Gliding.Diving", "Actor is diving forward in high-speed glide mode");

	// Grappling Hook & Swing Tags
	Instance.AddTag(Instance.State_Movement_Grappling, "State.Movement.Grappling", "Actor is actively anchored with grappling hook");
	Instance.AddTag(Instance.State_Movement_Grappling_Pulling, "State.Movement.Grappling.Pulling", "Actor is being rapidly pulled towards grapple anchor");
	Instance.AddTag(Instance.State_Movement_Grappling_Swinging, "State.Movement.Grappling.Swinging", "Actor is physically swinging on grapple cable");

	// Zipline & Sliding Cable Tags
	Instance.AddTag(Instance.State_Movement_Ziplining, "State.Movement.Ziplining", "Actor is actively riding or attached to a zipline cable");
	Instance.AddTag(Instance.State_Movement_Ziplining_Sliding, "State.Movement.Ziplining.Sliding", "Actor is actively sliding along the zipline cable");
	Instance.AddTag(Instance.State_Movement_Ziplining_Dismounting, "State.Movement.Ziplining.Dismounting", "Actor is transitioning off the zipline with launch momentum");

	// Vehicle & Driving Tags
	Instance.AddTag(Instance.State_Movement_Driving, "State.Movement.Driving", "Actor is actively driving a vehicle as pilot/driver");
	Instance.AddTag(Instance.State_Movement_Driving_Accelerating, "State.Movement.Driving.Accelerating", "Vehicle throttle is active and vehicle is accelerating");
	Instance.AddTag(Instance.State_Movement_Driving_Braking, "State.Movement.Driving.Braking", "Vehicle braking or handbrake is engaged");
	Instance.AddTag(Instance.State_Movement_Driving_Reverse, "State.Movement.Driving.Reverse", "Vehicle is actively traveling in reverse gear");
	Instance.AddTag(Instance.State_Vehicle_Occupied, "State.Vehicle.Occupied", "Vehicle currently has one or more passenger/driver occupants");
	Instance.AddTag(Instance.State_Vehicle_EngineRunning, "State.Vehicle.EngineRunning", "Vehicle engine ignition is active and running");

	// Watercraft & Sailing Tags
	Instance.AddTag(Instance.State_Movement_Sailing, "State.Movement.Sailing", "Actor is actively sailing or piloting a watercraft");
	Instance.AddTag(Instance.State_Movement_Sailing_Cruising, "State.Movement.Sailing.Cruising", "Watercraft is actively cruising with propeller/sail thrust");
	Instance.AddTag(Instance.State_Movement_Sailing_Anchored, "State.Movement.Sailing.Anchored", "Watercraft has dropped anchor and is moored in place");
	Instance.AddTag(Instance.State_Vehicle_Watercraft, "State.Vehicle.Watercraft", "Actor is classified as a marine/river watercraft vessel");

	// Aircraft & Flight Dynamics Tags
	Instance.AddTag(Instance.State_Movement_Flying, "State.Movement.Flying", "Actor is actively flying or piloting an aircraft");
	Instance.AddTag(Instance.State_Movement_Flying_Airborne, "State.Movement.Flying.Airborne", "Aircraft has generated sufficient lift and is airborne in flight");
	Instance.AddTag(Instance.State_Movement_Flying_Stalling, "State.Movement.Flying.Stalling", "Aircraft airspeed dropped below stall speed resulting in loss of lift");
	Instance.AddTag(Instance.State_Movement_Flying_VTOL, "State.Movement.Flying.VTOL", "Aircraft is operating in vertical takeoff and landing or hover rotor mode");
	Instance.AddTag(Instance.State_Vehicle_Aircraft, "State.Vehicle.Aircraft", "Actor is classified as an airborne aircraft flight vessel");

	// Spacecraft & 6-DOF Zero-G Dynamics Tags
	Instance.AddTag(Instance.State_Movement_Spaceflight, "State.Movement.Spaceflight", "Actor is actively piloting or commanding a spacecraft");
	Instance.AddTag(Instance.State_Movement_Spaceflight_Cruising, "State.Movement.Spaceflight.Cruising", "Spacecraft is applying active propulsion thrusters");
	Instance.AddTag(Instance.State_Movement_Spaceflight_FlightAssistOff, "State.Movement.Spaceflight.FlightAssistOff", "Spacecraft inertia dampeners are disabled for pure Newtonian drift");
	Instance.AddTag(Instance.State_Movement_Spaceflight_Reentry, "State.Movement.Spaceflight.Reentry", "Spacecraft is undergoing atmospheric re-entry friction and thermal heating");
	Instance.AddTag(Instance.State_Vehicle_Spacecraft, "State.Vehicle.Spacecraft", "Actor is classified as an orbital or deep space vehicle");

	// Mech & Exosuit Locomotion Tags
	Instance.AddTag(Instance.State_Movement_Mech, "State.Movement.Mech", "Actor is actively piloting or operating a heavy mech or exosuit");
	Instance.AddTag(Instance.State_Movement_Mech_Walking, "State.Movement.Mech.Walking", "Mech is engaged in heavy bipedal stride locomotion");
	Instance.AddTag(Instance.State_Movement_Mech_JumpJets, "State.Movement.Mech.JumpJets", "Mech vertical thrusters / jump jets are actively firing");
	Instance.AddTag(Instance.State_Movement_Mech_Overheated, "State.Movement.Mech.Overheated", "Mech power core exceeded safe temperature threshold and entered emergency shutdown");
	Instance.AddTag(Instance.State_Vehicle_Mech, "State.Vehicle.Mech", "Actor is classified as a bipedal robotic mech or exosuit");

	// Heavy Machinery & Hydraulic Physics Tags
	Instance.AddTag(Instance.State_Movement_Machinery, "State.Movement.Machinery", "Actor is actively operating heavy construction or mining machinery");
	Instance.AddTag(Instance.State_Movement_Machinery_Operating, "State.Movement.Machinery.Operating", "Machinery hydraulic circuits and boom/arm actuators are active");
	Instance.AddTag(Instance.State_Movement_Machinery_Lifting, "State.Movement.Machinery.Lifting", "Crane winch or boom is actively hoisting suspended cargo");
	Instance.AddTag(Instance.State_Movement_Machinery_Excavating, "State.Movement.Machinery.Excavating", "Excavator bucket or earthmoving blade is actively engaging terrain");
	Instance.AddTag(Instance.State_Vehicle_Machinery, "State.Vehicle.Machinery", "Actor is classified as heavy civil, industrial or mining machinery");

	// Modular Structural Integrity & Collapse Tags
	Instance.AddTag(Instance.State_Building_Anchor, "State.Building.Anchor", "Building piece is firmly anchored to ground with infinite foundational support");
	Instance.AddTag(Instance.State_Building_Supported, "State.Building.Supported", "Building piece has a valid structural load path connecting to a ground anchor");
	Instance.AddTag(Instance.State_Building_Stressed, "State.Building.Stressed", "Building piece is under severe mechanical load or approaching maximum structural capacity");
	Instance.AddTag(Instance.State_Building_Collapsing, "State.Building.Collapsing", "Building piece lost foundational support and is undergoing physics-based collapse");

	// Power Grid & Electric Circuit Tags
	Instance.AddTag(Instance.State_Power_Powered, "State.Power.Powered", "Machine or consumer is receiving adequate electric power");
	Instance.AddTag(Instance.State_Power_Unpowered, "State.Power.Unpowered", "Machine or consumer is unpowered or experiencing a blackout");
	Instance.AddTag(Instance.State_Power_Overloaded, "State.Power.Overloaded", "Power grid demand exceeded capacity and tripped circuit breakers");
	Instance.AddTag(Instance.State_Power_Charging, "State.Power.Charging", "Battery or accumulator is absorbing surplus power from the grid");
	Instance.AddTag(Instance.State_Power_Discharging, "State.Power.Discharging", "Battery or accumulator is discharging stored energy into the grid");

	// Pipe Networks, Fluids & Gas Mechanics Tags
	Instance.AddTag(Instance.State_Fluid_Flowing, "State.Fluid.Flowing", "Pipe or conduit is actively transporting liquid or gas without obstruction");
	Instance.AddTag(Instance.State_Fluid_Blocked, "State.Fluid.Blocked", "Fluid flow is blocked by closed valve or terminal dead end");
	Instance.AddTag(Instance.State_Fluid_Pressurized, "State.Fluid.Pressurized", "Pipe network is operating under active pump pressure or hydrostatic gradient");
	Instance.AddTag(Instance.State_Fluid_Leaking, "State.Fluid.Leaking", "Pipe segment or valve is leaking fluid or venting gas into surrounding environment");
	Instance.AddTag(Instance.State_Fluid_Ruptured, "State.Fluid.Ruptured", "Pipe segment suffered structural breach due to extreme overpressure");

	// Conveyor Belts & Factory Logistics Tags
	Instance.AddTag(Instance.State_Logistics_Conveying, "State.Logistics.Conveying", "Conveyor belt is actively transporting items along its track");
	Instance.AddTag(Instance.State_Logistics_Jammed, "State.Logistics.Jammed", "Conveyor belt is jammed or paused due to downstream backpressure");
	Instance.AddTag(Instance.State_Logistics_Sorting, "State.Logistics.Sorting", "Smart sorter is inspecting and filtering items based on gameplay tags");
	Instance.AddTag(Instance.State_Logistics_Merging, "State.Logistics.Merging", "Conveyor merger is unifying multiple input streams into a single output track");

	// Industrial Processing & Machinery Tags
	Instance.AddTag(Instance.State_Industrial_Processing, "State.Industrial.Processing", "Industrial machine is actively processing ingredients into crafted outputs");
	Instance.AddTag(Instance.State_Industrial_Idle, "State.Industrial.Idle", "Industrial machine is in idle state waiting for production orders");
	Instance.AddTag(Instance.State_Industrial_MissingIngredients, "State.Industrial.MissingIngredients", "Industrial machine lacks required input items or fluids");
	Instance.AddTag(Instance.State_Industrial_NoPower, "State.Industrial.NoPower", "Industrial machine is starved of required electric power");
	Instance.AddTag(Instance.State_Industrial_OutputFull, "State.Industrial.OutputFull", "Industrial machine output buffer is full and cannot dump finished product");
	Instance.AddTag(Instance.State_Industrial_Overheated, "State.Industrial.Overheated", "Industrial machine exceeded safe operating thermal threshold");

	// Resource Extractors, Mining Drills & Wells Tags
	Instance.AddTag(Instance.State_Extractor_Drilling, "State.Extractor.Drilling", "Resource extractor or drill is actively harvesting raw resources from node");
	Instance.AddTag(Instance.State_Extractor_Idle, "State.Extractor.Idle", "Resource extractor is idle waiting for target geological resource node");
	Instance.AddTag(Instance.State_Extractor_Depleted, "State.Extractor.Depleted", "Target resource deposit is completely depleted and cannot yield further output");
	Instance.AddTag(Instance.State_Extractor_NoPower, "State.Extractor.NoPower", "Resource extractor is starved of required electrical power");
	Instance.AddTag(Instance.State_Extractor_OutputBlocked, "State.Extractor.OutputBlocked", "Resource extractor output buffer or connected line is full");
	Instance.AddTag(Instance.State_Extractor_Overheated, "State.Extractor.Overheated", "Resource extractor drill head exceeded safe thermal operating limit");

	// Automated Freight Trains & Railroad Logistics Tags
	Instance.AddTag(Instance.State_Rail_Traveling, "State.Rail.Traveling", "Freight train is actively traveling along railroad track");
	Instance.AddTag(Instance.State_Rail_Loading, "State.Rail.Loading", "Freight train is stopped at platform loading cargo wagons");
	Instance.AddTag(Instance.State_Rail_Unloading, "State.Rail.Unloading", "Freight train is stopped at platform unloading cargo wagons");
	Instance.AddTag(Instance.State_Rail_WaitingSignal, "State.Rail.WaitingSignal", "Train is halted before an occupied rail block waiting for green signal");
	Instance.AddTag(Instance.State_Rail_Derailed, "State.Rail.Derailed", "Train consist has derailed from track following collision or switch failure");

	// Programmable Logic Controllers (PLC) & Circuit Networks Tags
	Instance.AddTag(Instance.State_Logic_Evaluating, "State.Logic.Evaluating", "Logic node is actively computing circuit signal expressions");
	Instance.AddTag(Instance.State_Logic_ConditionMet, "State.Logic.ConditionMet", "Logic condition, comparator threshold, or boolean gate evaluated to true");
	Instance.AddTag(Instance.State_Logic_ConditionFailed, "State.Logic.ConditionFailed", "Logic condition or comparator evaluated to false");
	Instance.AddTag(Instance.State_Logic_Pulsing, "State.Logic.Pulsing", "Logic node is generating an active clock cycle or discrete signal pulse");
	Instance.AddTag(Instance.State_Logic_Disabled, "State.Logic.Disabled", "Logic node is deactivated or lacking electrical circuit power");

	// Industrial Cargo Drones & Sky Corridors Tags
	Instance.AddTag(Instance.State_Drone_Idle, "State.Drone.Idle", "Cargo drone is resting idle on port docking pad");
	Instance.AddTag(Instance.State_Drone_TakingOff, "State.Drone.TakingOff", "Cargo drone is ascending vertically to cruise altitude");
	Instance.AddTag(Instance.State_Drone_InFlight, "State.Drone.InFlight", "Cargo drone is actively flying along sky route towards destination port");
	Instance.AddTag(Instance.State_Drone_Landing, "State.Drone.Landing", "Cargo drone is descending vertically to destination port landing pad");
	Instance.AddTag(Instance.State_Drone_Recharging, "State.Drone.Recharging", "Cargo drone is docked and actively replenishing battery power");
	Instance.AddTag(Instance.State_Drone_LowBattery, "State.Drone.LowBattery", "Drone battery has reached critical reserve threshold initiating return to base");

	// Modular Space Elevator & Planetary Logistics Tags
	Instance.AddTag(Instance.State_SpaceElevator_Idle, "State.SpaceElevator.Idle", "Space elevator is awaiting cargo deposit at terrestrial base");
	Instance.AddTag(Instance.State_SpaceElevator_Ascending, "State.SpaceElevator.Ascending", "Space elevator payload pod is ascending orbital tether cable");
	Instance.AddTag(Instance.State_SpaceElevator_Descending, "State.SpaceElevator.Descending", "Space elevator payload pod is descending from orbit back to terrestrial base");
	Instance.AddTag(Instance.State_SpaceElevator_PhaseCompleted, "State.SpaceElevator.PhaseCompleted", "Space elevator project phase has been completed unlocking higher tier");
	Instance.AddTag(Instance.State_SpaceElevator_Delivering, "State.SpaceElevator.Delivering", "Space elevator is actively dispatching planetary payload to low orbit");

	// Thermal Dynamics, Body Temperature & Hypothermia Tags
	Instance.AddTag(Instance.State_Thermal_Comfortable, "State.Thermal.Comfortable", "Body core temperature is in homeostatic equilibrium");
	Instance.AddTag(Instance.State_Thermal_Cold, "State.Thermal.Cold", "Body temperature is declining causing shivering and increased calorie burn");
	Instance.AddTag(Instance.State_Thermal_Freezing, "State.Thermal.Freezing", "Body temperature is dangerously low approaching freezing limit");
	Instance.AddTag(Instance.State_Thermal_Warm, "State.Thermal.Warm", "Body temperature is elevating with light perspiration");
	Instance.AddTag(Instance.State_Thermal_Overheating, "State.Thermal.Overheating", "Body temperature is high causing heavy sweating and rapid dehydration");
	Instance.AddTag(Instance.State_Thermal_Hypothermia, "State.Thermal.Hypothermia", "Severe hypothermia has occurred causing continuous health damage");
	Instance.AddTag(Instance.State_Thermal_Heatstroke, "State.Thermal.Heatstroke", "Severe heatstroke has occurred causing dizziness and thermal damage");

	// Advanced Metabolic Nutrition & Deficiency Tags
	Instance.AddTag(Instance.State_Metabolism_WellFed, "State.Metabolism.WellFed", "Character has ample caloric reserves and maximum physical stamina efficiency");
	Instance.AddTag(Instance.State_Metabolism_Hungry, "State.Metabolism.Hungry", "Character has moderate caloric deficit experiencing mild hunger");
	Instance.AddTag(Instance.State_Metabolism_Starving, "State.Metabolism.Starving", "Character has exhausted caloric reserves suffering starvation health decay");
	Instance.AddTag(Instance.State_Metabolism_Hydrated, "State.Metabolism.Hydrated", "Character has optimal hydration level");
	Instance.AddTag(Instance.State_Metabolism_Thirsty, "State.Metabolism.Thirsty", "Character is experiencing thirst and diminished stamina regeneration");
	Instance.AddTag(Instance.State_Metabolism_Dehydrated, "State.Metabolism.Dehydrated", "Character is severely dehydrated suffering critical health damage");
	Instance.AddTag(Instance.State_Metabolism_Deficiency_VitaminC, "State.Metabolism.Deficiency.VitaminC", "Character is suffering from Vitamin C deficiency (Scurvy) causing bleeding and weakness");
	Instance.AddTag(Instance.State_Metabolism_Deficiency_VitaminA, "State.Metabolism.Deficiency.VitaminA", "Character is suffering from Vitamin A deficiency causing night blindness and perception penalty");
	Instance.AddTag(Instance.State_Metabolism_Deficiency_Electrolytes, "State.Metabolism.Deficiency.Electrolytes", "Character has severe electrolyte depletion causing frequent muscle cramps");

	// Advanced Pathogen, Infection & Immune System Tags
	Instance.AddTag(Instance.State_Immunity_Infected, "State.Immunity.Infected", "Character is currently hosting an active pathogen infection");
	Instance.AddTag(Instance.State_Immunity_Incubating, "State.Immunity.Incubating", "Pathogen infection is in early asymptomatic incubation phase");
	Instance.AddTag(Instance.State_Immunity_Fever, "State.Immunity.Fever", "Body has triggered reactive fever elevating core temperature to fight infection");
	Instance.AddTag(Instance.State_Immunity_Symptomatic, "State.Immunity.Symptomatic", "Infection has surpassed symptomatic threshold causing illness and weakness");
	Instance.AddTag(Instance.State_Immunity_Recovering, "State.Immunity.Recovering", "Immune antibodies and medicine have suppressed pathogen into convalescence");
	Instance.AddTag(Instance.State_Immunity_Immune, "State.Immunity.Immune", "Character has acquired adaptive biological immunity against the pathogen strain");

	// Advanced Physical Trauma, Hemorrhage & Fractures Tags
	Instance.AddTag(Instance.State_Trauma_Bleeding, "State.Trauma.Bleeding", "Character is suffering from active venous or minor blood loss");
	Instance.AddTag(Instance.State_Trauma_ArterialBleed, "State.Trauma.ArterialBleed", "Character is suffering from severe arterial hemorrhaging requiring tourniquet or surgical intervention");
	Instance.AddTag(Instance.State_Trauma_Fracture_Arm, "State.Trauma.Fracture.Arm", "Character has fractured arm bones suffering severe attack and handling penalties");
	Instance.AddTag(Instance.State_Trauma_Fracture_Leg, "State.Trauma.Fracture.Leg", "Character has fractured leg bones suffering severe locomotion speed penalties");
	Instance.AddTag(Instance.State_Trauma_TourniquetApplied, "State.Trauma.TourniquetApplied", "Character has a tourniquet applied on a limb to halt arterial bleeding");
	Instance.AddTag(Instance.State_Trauma_HypovolemicShock, "State.Trauma.HypovolemicShock", "Character has critically low blood volume suffering dizziness and hypovolemic shock");

	// Advanced Dynamic Flora & Agriculture Tags
	Instance.AddTag(Instance.State_Crop_Seeded, "State.Crop.Seeded", "Crop plot has viable seed planted in germination stage");
	Instance.AddTag(Instance.State_Crop_Sprouting, "State.Crop.Sprouting", "Crop plot has sprouted green shoots above soil");
	Instance.AddTag(Instance.State_Crop_Growing, "State.Crop.Growing", "Crop is actively in vegetative foliage growth cycle");
	Instance.AddTag(Instance.State_Crop_Harvestable, "State.Crop.Harvestable", "Crop has matured and is ready for agricultural harvest");
	Instance.AddTag(Instance.State_Crop_Withered, "State.Crop.Withered", "Crop has dried out or frozen and withered away");
	Instance.AddTag(Instance.State_Crop_Fertilized, "State.Crop.Fertilized", "Soil plot has active fertilizer enriching growth speed and yield");

	// Advanced Fauna Ecosystem, Domestication & Genetics Tags
	Instance.AddTag(Instance.State_Fauna_Wild, "State.Fauna.Wild", "Creature is currently wild and un-tamed");
	Instance.AddTag(Instance.State_Fauna_Taming, "State.Fauna.Taming", "Creature is undergoing taming and domestication habituation");
	Instance.AddTag(Instance.State_Fauna_Domesticated, "State.Fauna.Domesticated", "Creature is fully domesticated and loyal to handler");
	Instance.AddTag(Instance.State_Fauna_Pregnant, "State.Fauna.Pregnant", "Creature is pregnant and carrying gestating offspring");
	Instance.AddTag(Instance.State_Fauna_Juvenile, "State.Fauna.Juvenile", "Creature is a juvenile growing into adult specimen");
	Instance.AddTag(Instance.State_Fauna_Mountable, "State.Fauna.Mountable", "Creature has high affection and is rideable as mount");

	// Advanced Planetary Atmosphere & Toxic Gas Hazard Tags
	Instance.AddTag(Instance.State_Atmosphere_Hazardous, "State.Atmosphere.Hazardous", "Atmospheric environment is dangerous to breathe without protection");
	Instance.AddTag(Instance.State_Atmosphere_Hypoxia, "State.Atmosphere.Hypoxia", "Character has low blood oxygen saturation suffering hypoxia and asphyxiation");
	Instance.AddTag(Instance.State_Atmosphere_ToxicInhalation, "State.Atmosphere.ToxicInhalation", "Character is actively inhaling poisonous atmospheric toxins or chemical gas");
	Instance.AddTag(Instance.State_Atmosphere_SuitPressurized, "State.Atmosphere.SuitPressurized", "Suit is hermetically sealed and providing oxygen under pressurized life support");
	Instance.AddTag(Instance.State_Atmosphere_FilterExhausted, "State.Atmosphere.FilterExhausted", "Gas mask or suit air filter is fully clogged and exhausted");
	Instance.AddTag(Instance.State_Atmosphere_Decompression, "State.Atmosphere.Decompression", "Character is undergoing rapid explosive decompression barotrauma");

	// Advanced Nuclear Radiation & Dosimetry Tags
	Instance.AddTag(Instance.State_Radiation_Exposed, "State.Radiation.Exposed", "Character is exposed to ionizing radiation dosage");
	Instance.AddTag(Instance.State_Radiation_LowDose, "State.Radiation.LowDose", "Character has accumulated low ionizing radiation dosage suffering mild nausea");
	Instance.AddTag(Instance.State_Radiation_AcuteSickness, "State.Radiation.AcuteSickness", "Character suffers Acute Radiation Sickness with immune cell breakdown");
	Instance.AddTag(Instance.State_Radiation_CriticalARS, "State.Radiation.CriticalARS", "Character is suffering lethal Acute Radiation Sickness with organ failure");
	Instance.AddTag(Instance.State_Radiation_LeadShielded, "State.Radiation.LeadShielded", "Character is protected by high density lead radiation shielding");
	Instance.AddTag(Instance.State_Radiation_GeigerClicking, "State.Radiation.GeigerClicking", "Geiger counter is actively clicking due to high ambient radiation flux");

	// Advanced Afflictions & Bio-Compounds Tags
	Instance.AddTag(Instance.State_Affliction_Impaired, "State.Affliction.Impaired", "Character is suffering from motor locomotion impairment");
	Instance.AddTag(Instance.State_Affliction_Degradation, "State.Affliction.Degradation", "Character is undergoing physical tissue degradation affliction");
	Instance.AddTag(Instance.State_Affliction_Paralyzed, "State.Affliction.Paralyzed", "Character motor movement is fully paralyzed by severe affliction");
	Instance.AddTag(Instance.State_Affliction_Inoculated, "State.Affliction.Inoculated", "Character has acquired biological resistance against afflictions");
	Instance.AddTag(Instance.State_Affliction_Neutralized, "State.Affliction.Neutralized", "Affliction was successfully cured and neutralized by elixir");

	// Advanced Surgery, Prosthetics & Organ Transplants Tags
	Instance.AddTag(Instance.State_Surgery_UnderAnesthesia, "State.Surgery.UnderAnesthesia", "Patient is sedated under surgical anesthesia");
	Instance.AddTag(Instance.State_Surgery_Operating, "State.Surgery.Operating", "Patient is currently undergoing an active surgical operation");
	Instance.AddTag(Instance.State_Surgery_ProstheticInstalled, "State.Surgery.ProstheticInstalled", "Patient has one or more prosthetic limbs installed");
	Instance.AddTag(Instance.State_Surgery_OrganRejection, "State.Surgery.OrganRejection", "Patient is experiencing organ transplant tissue rejection");
	Instance.AddTag(Instance.State_Surgery_CyberneticAugmented, "State.Surgery.CyberneticAugmented", "Patient is augmented with advanced cybernetic prosthetics");

	// Dynamic Tick Throttling & Hierarchical LOD Tags
	Instance.AddTag(Instance.State_Throttling_LOD0, "State.Throttling.LOD0", "Entity is executing at full tick rate LOD0 (High Priority)");
	Instance.AddTag(Instance.State_Throttling_LOD1, "State.Throttling.LOD1", "Entity is executing at throttled rate LOD1 (Medium Priority)");
	Instance.AddTag(Instance.State_Throttling_LOD2, "State.Throttling.LOD2", "Entity is executing at throttled rate LOD2 (Low Priority)");
	Instance.AddTag(Instance.State_Throttling_Background, "State.Throttling.Background", "Entity is executing in background batch mode LOD3");
	Instance.AddTag(Instance.State_Throttling_Suspended, "State.Throttling.Suspended", "Entity ticking is suspended/dormant");

	// Memory Layout & Cache Locality Tags
	Instance.AddTag(Instance.State_Memory_Optimized, "State.Memory.Optimized", "Entity data is packed and aligned in optimized cache structure");
	Instance.AddTag(Instance.State_Memory_Contiguous, "State.Memory.Contiguous", "Entity is located inside a contiguous flat memory buffer");
	Instance.AddTag(Instance.State_Memory_ZeroAllocActive, "State.Memory.ZeroAllocActive", "Entity is participating in zero-allocation iteration loop");

	// Multi-Threading & Async Task Graph Tags
	Instance.AddTag(Instance.State_Async_TaskRunning, "State.Async.TaskRunning", "Entity is running background async computation task");
	Instance.AddTag(Instance.State_Async_DoubleBufferActive, "State.Async.DoubleBufferActive", "Entity uses safe double-buffered data synchronization");
	Instance.AddTag(Instance.State_Async_WorkCompleted, "State.Async.WorkCompleted", "Background async computation task completed successfully");

	// Hierarchical Spatial Partitioning & Grid Tags
	Instance.AddTag(Instance.State_Spatial_Indexed, "State.Spatial.Indexed", "Entity is indexed in the spatial partitioning grid");
	Instance.AddTag(Instance.State_Spatial_CellActive, "State.Spatial.CellActive", "Entity is inside an active spatial cell");
	Instance.AddTag(Instance.State_Spatial_Queried, "State.Spatial.Queried", "Entity matched a spatial range or box query");

	// Lock-Free Concurrency & Ring Buffer Tags
	Instance.AddTag(Instance.State_LockFree_Active, "State.LockFree.Active", "Lock-free messaging and atomic event queuing is active");
	Instance.AddTag(Instance.State_LockFree_Buffering, "State.LockFree.Buffering", "Events are actively buffering in the lock-free ring queue");
	Instance.AddTag(Instance.State_LockFree_Drained, "State.LockFree.Drained", "Lock-free ring queue has been fully drained by consumer");

	// Event Bus Centralized Pub/Sub Tags
	Instance.AddTag(Instance.Event_Combat_DamageDealt, "Event.Combat.DamageDealt", "Event channel notifying combat damage dealt");
	Instance.AddTag(Instance.Event_Inventory_ItemCrafted, "Event.Inventory.ItemCrafted", "Event channel notifying successful item craft");
	Instance.AddTag(Instance.Event_Survival_StatusChanged, "Event.Survival.StatusChanged", "Event channel notifying survival status modification");

	// Async Serialization & Binary Persistence Tags
	Instance.AddTag(Instance.State_Save_AsyncSaving, "State.Save.AsyncSaving", "Entity state is currently being serialized and saved asynchronously in background");
	Instance.AddTag(Instance.State_Save_AsyncLoading, "State.Save.AsyncLoading", "Entity state is currently being deserialized and loaded asynchronously");
	Instance.AddTag(Instance.State_Save_Serialized, "State.Save.Serialized", "Entity has a valid serialized state snapshot");

	// Dynamic Fault Tolerance & Fallback Tags
	Instance.AddTag(Instance.State_Fault_Degraded, "State.Fault.Degraded", "Entity or service is currently operating in degraded performance mode");
	Instance.AddTag(Instance.State_Fault_FallbackActive, "State.Fault.FallbackActive", "Entity or service is currently using fallback default contingency values");
	Instance.AddTag(Instance.State_Fault_Resilient, "State.Fault.Resilient", "Entity is protected by fault tolerance and graceful degradation guards");

	// State Machine & Tag Matrix Verification Tags
	Instance.AddTag(Instance.State_Matrix_Verified, "State.Matrix.Verified", "Entity active state tag composition is formally verified by the matrix");
	Instance.AddTag(Instance.State_Matrix_ConflictDetected, "State.Matrix.ConflictDetected", "Entity had an incompatible state tag conflict intercepted");
	Instance.AddTag(Instance.State_Matrix_StrictEnforcement, "State.Matrix.StrictEnforcement", "Entity is under strict tag transition matrix rejection rules");

	// Live Reflection & Config Hot-Reloading Tags
	Instance.AddTag(Instance.State_Config_HotReloadActive, "State.Config.HotReloadActive", "Entity is currently undergoing live config schema hot-reloading");
	Instance.AddTag(Instance.State_Config_SchemaSynced, "State.Config.SchemaSynced", "Entity local parameters are fully synchronized with the live config schema");
	Instance.AddTag(Instance.State_Config_Observing, "State.Config.Observing", "Entity is actively observing runtime configuration schema updates");

	// In-Editor Visual Debugger & Viewport Overlay Tags
	Instance.AddTag(Instance.State_Debug_OverlayActive, "State.Debug.OverlayActive", "Entity is actively participating in visual viewport debug rendering");
	Instance.AddTag(Instance.State_Debug_VisualizingPower, "State.Debug.VisualizingPower", "Entity is visualizing power grid connections and potential");
	Instance.AddTag(Instance.State_Debug_VisualizingLogistics, "State.Debug.VisualizingLogistics", "Entity is visualizing logistics conveyance or drone routing paths");

	// Real-Time Performance Profiler Tags
	Instance.AddTag(Instance.State_Profiler_Instrumented, "State.Profiler.Instrumented", "Entity is actively instrumented by the real-time performance profiler");
	Instance.AddTag(Instance.State_Profiler_BudgetExceeded, "State.Profiler.BudgetExceeded", "Entity has exceeded its designated performance frame budget threshold");
	Instance.AddTag(Instance.State_Profiler_SamplingActive, "State.Profiler.SamplingActive", "Entity performance metrics are currently being sampled in real-time");

	// Procedural World Validation & Integrity Tags
	Instance.AddTag(Instance.State_Integrity_Audited, "State.Integrity.Audited", "Entity or asset has been formally verified by the world integrity subsystem");
	Instance.AddTag(Instance.State_Integrity_IssueDetected, "State.Integrity.IssueDetected", "Entity has an integrity inconsistency detected during validation audit");
	Instance.AddTag(Instance.State_Integrity_Clean, "State.Integrity.Clean", "Entity has passed all procedural world validation and integrity audits without error");

	// Automated Stress-Testing & Bot Swarm Tags
	Instance.AddTag(Instance.State_Stress_BotActive, "State.Stress.BotActive", "Simulated stress bot is currently active in the world");
	Instance.AddTag(Instance.State_Stress_SimulatingAction, "State.Stress.SimulatingAction", "Simulated stress bot is actively executing a benchmark workload action");
	Instance.AddTag(Instance.State_Stress_SwarmMember, "State.Stress.SwarmMember", "Entity is an active registered participant of the automated bot swarm stress test");

	// Smart Object Activity Tags
	Instance.AddTag(Instance.Activity_TestInteraction, "Activity.TestInteraction", "Activity Tag for test Smart Object interaction");
}

void FSBGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment)
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	OutTag = TagsManager.AddNativeGameplayTag(FName(TagName), FString(TagComment));
}
