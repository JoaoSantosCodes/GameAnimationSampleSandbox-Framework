// Copyright 2026 João Santos. All Rights Reserved.
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
	FGameplayTag State_Character_Frozen;
	FGameplayTag State_Character_Crouching;
	FGameplayTag State_Character_Exhausted;
	FGameplayTag State_Character_HitReacting;
	FGameplayTag State_Character_Encumbered;
	FGameplayTag State_Weapon_Firing;
	FGameplayTag State_Item_Equipped;

	// Movement Actions
	FGameplayTag Movement_Action_Sprint;

	// Inputs
	FGameplayTag Input_Action_Move;
	FGameplayTag Input_Action_Look;
	FGameplayTag Input_Action_Jump;
	FGameplayTag Input_Action_Sprint;
	FGameplayTag Input_Action_Fire;
	FGameplayTag Input_Action_Reload;
	FGameplayTag Input_Action_Interact;

	// Events - Character & Combat
	FGameplayTag Event_Character_Damaged;
	FGameplayTag Event_Character_Dead;
	FGameplayTag Event_Character_Footstep;
	FGameplayTag Event_Weapon_Fire;
	FGameplayTag Event_Combat_HitReact;
	FGameplayTag Event_Combat_CriticalHit;
	FGameplayTag Combat_Action_Reload;
	FGameplayTag Combat_Action_Fire;

	// Events - Attributes
	FGameplayTag Event_Attribute_Changed;

	// Events - Interaction
	FGameplayTag Event_Interaction_Available;
	FGameplayTag Event_Interaction_Cleared;
	FGameplayTag Event_Interaction_Progress;
	FGameplayTag Event_Interaction_Started;
	FGameplayTag Event_Interaction_Completed;

	// Events - Area
	FGameplayTag Event_Area_Discovered;

	// Features
	FGameplayTag Feature_Combat;
	FGameplayTag Feature_Inventory;
	FGameplayTag Feature_Crafting;
	FGameplayTag Feature_Building;
	FGameplayTag Feature_Quests;

	// Feature Events
	FGameplayTag Event_Feature_Toggled;

	// Background Simulation Tags
	FGameplayTag SimType_ResourceNode;
	FGameplayTag State_Timer_Respawn;

	// Events - Inventory
	FGameplayTag Event_Inventory_ItemAdded;
	FGameplayTag Event_Inventory_ItemRemoved;
	FGameplayTag Event_Inventory_ItemEquipped;
	FGameplayTag Event_Inventory_ItemUnequipped;
	FGameplayTag Event_Inventory_SlotUpdated;

	// Events - Abilities
	FGameplayTag Event_Ability_CooldownStarted;
	FGameplayTag Event_Ability_CooldownEnded;

	// Events - Quests
	FGameplayTag Event_Quest_RewardsClaimed;

	// Attributes
	FGameplayTag Attribute_Health;
	FGameplayTag Attribute_MaxHealth;
	FGameplayTag Attribute_Stamina;
	FGameplayTag Attribute_Mana;
	FGameplayTag Attribute_Speed;
	FGameplayTag Attribute_Defense;
	FGameplayTag Attribute_Weapon_Ammo;
	FGameplayTag Attribute_Coins;
	FGameplayTag Attribute_Weight;
	FGameplayTag Attribute_MaxWeight;

	// Equipment Slots
	FGameplayTag Equipment_Slot_Head;
	FGameplayTag Equipment_Slot_Chest;
	FGameplayTag Equipment_Slot_Feet;
	FGameplayTag Equipment_Slot_Hands;
	FGameplayTag Equipment_Slot_Ring;

	// Crafting & Stations
	FGameplayTag Crafting_Station_Forge;
	FGameplayTag Crafting_Station_Alchemy;
	FGameplayTag Event_Crafting_Completed;
	FGameplayTag Event_Crafting_Failed;

	// Quest & Items Mock Types
	FGameplayTag Item_Type_Material;
	FGameplayTag Quest_Objective_Footstep;

	// Loot Rarity Tags
	FGameplayTag Loot_Rarity_Common;
	FGameplayTag Loot_Rarity_Uncommon;
	FGameplayTag Loot_Rarity_Rare;
	FGameplayTag Loot_Rarity_Epic;
	FGameplayTag Loot_Rarity_Legendary;

	// Region and Zone Types
	FGameplayTag Zone_Type_SafeZone;
	FGameplayTag Zone_Type_PvP;
	FGameplayTag Zone_Type_Dungeon;
	FGameplayTag Zone_Type_Hazard;

	// Zone State Tags
	FGameplayTag State_Zone_Safe;
	FGameplayTag State_Zone_PvPAllowed;
	FGameplayTag State_Zone_InHazard;

	// Weather State Tags
	FGameplayTag State_Weather_Clear;
	FGameplayTag State_Weather_Rain;
	FGameplayTag State_Weather_Snow;
	FGameplayTag State_Weather_Storm;
	FGameplayTag State_Weather_Fog;

	// Time Period State Tags
	FGameplayTag State_Time_Dawn;
	FGameplayTag State_Time_Day;
	FGameplayTag State_Time_Dusk;
	FGameplayTag State_Time_Night;

	// Portal Types
	FGameplayTag Portal_Type_Gateway;
	FGameplayTag Portal_Type_Waystone;
	FGameplayTag Portal_Type_DungeonGate;

	// Portal State Tags
	FGameplayTag State_Portal_Teleporting;
	FGameplayTag State_Portal_Locked;
	FGameplayTag State_Portal_Cooldown;

	// Gameplay Effect Tags
	FGameplayTag Effect_Buff_Berserk;
	FGameplayTag Effect_Buff_SpeedBoost;
	FGameplayTag Effect_Buff_Regeneration;
	FGameplayTag Effect_Debuff_Poison;
	FGameplayTag Effect_Debuff_Burn;
	FGameplayTag Effect_Debuff_Slow;
	FGameplayTag Effect_Debuff_Stun;

	// Immunity State Tags
	FGameplayTag State_Immunity_Poison;
	FGameplayTag State_Immunity_Stun;
	FGameplayTag State_Immunity_Burn;

	// Combo Tags
	FGameplayTag Combat_Combo_Light;
	FGameplayTag Combat_Combo_Heavy;
	FGameplayTag Combat_Combo_Finisher;

	// Combo State Tags
	FGameplayTag State_Combat_ComboWindowOpen;
	FGameplayTag State_Combat_FinisherReady;

	// Defense & Parry Tags
	FGameplayTag State_Combat_Blocking;
	FGameplayTag State_Combat_ParryWindow;
	FGameplayTag State_Combat_GuardBroken;
	FGameplayTag State_Combat_CounterAttackReady;
	FGameplayTag State_Combat_Staggered;

	// Lock-On Tags
	FGameplayTag State_Combat_LockedOn;
	FGameplayTag State_Combat_Target;

	// Hit Trace & Melee Attack Tags
	FGameplayTag State_Combat_Attacking;
	FGameplayTag State_Combat_HitTraceActive;

	// Poise & Hit Reaction Tags
	FGameplayTag State_Combat_SuperArmor;
	FGameplayTag State_Combat_PoiseBroken;
	FGameplayTag Combat_Reaction_Front;
	FGameplayTag Combat_Reaction_Back;
	FGameplayTag Combat_Reaction_Left;
	FGameplayTag Combat_Reaction_Right;

	// Motion Warping Tags
	FGameplayTag State_Combat_MotionWarping;

	// Execution & Finisher Tags
	FGameplayTag State_Combat_Executing;
	FGameplayTag State_Combat_Executed;
	FGameplayTag State_Combat_Invulnerable;

	// Combat FX Tags
	FGameplayTag State_Combat_WeaponTrailActive;

	// Combat Feedback Tags
	FGameplayTag State_Combat_HitStop;
	FGameplayTag State_Combat_Slomo;

	// Dismemberment Tags
	FGameplayTag State_Combat_Dismembered;
	FGameplayTag Combat_Dismember_Head;
	FGameplayTag Combat_Dismember_Arm;
	FGameplayTag Combat_Dismember_Leg;

	// Stealth & Perception Tags
	FGameplayTag State_Combat_Stealth_Hidden;
	FGameplayTag State_Combat_Stealth_Suspicious;
	FGameplayTag State_Combat_Stealth_Detected;

	// Cover System Tags
	FGameplayTag State_Combat_InCover;
	FGameplayTag State_Combat_InCover_Low;
	FGameplayTag State_Combat_InCover_High;
	FGameplayTag State_Combat_Peeking;

	// Parkour & Locomotion Tags
	FGameplayTag State_Movement_ParkourActive;
	FGameplayTag State_Movement_Vaulting;
	FGameplayTag State_Movement_Mantling;

	// Foot IK & Grounding Tags
	FGameplayTag State_Movement_FootIKActive;
	FGameplayTag State_Movement_OnSlope;

	// Mount & Riding Tags
	FGameplayTag State_Movement_Mounted;
	FGameplayTag State_Movement_Mounting;
	FGameplayTag State_Movement_Dismounting;
	FGameplayTag State_Movement_Galloping;

	// Swimming & Water Locomotion Tags
	FGameplayTag State_Movement_Swimming;
	FGameplayTag State_Movement_Swimming_Surface;
	FGameplayTag State_Movement_Swimming_Diving;
	FGameplayTag State_Status_Drowning;

	// Gliding & Aerial Locomotion Tags
	FGameplayTag State_Movement_Gliding;
	FGameplayTag State_Movement_Gliding_Deploying;
	FGameplayTag State_Movement_Gliding_Diving;

	// Grappling Hook & Swing Tags
	FGameplayTag State_Movement_Grappling;
	FGameplayTag State_Movement_Grappling_Pulling;
	FGameplayTag State_Movement_Grappling_Swinging;

	// Zipline & Sliding Cable Tags
	FGameplayTag State_Movement_Ziplining;
	FGameplayTag State_Movement_Ziplining_Sliding;
	FGameplayTag State_Movement_Ziplining_Dismounting;

	// Vehicle & Driving Tags
	FGameplayTag State_Movement_Driving;
	FGameplayTag State_Movement_Driving_Accelerating;
	FGameplayTag State_Movement_Driving_Braking;
	FGameplayTag State_Movement_Driving_Reverse;
	FGameplayTag State_Vehicle_Occupied;
	FGameplayTag State_Vehicle_EngineRunning;

	// Watercraft & Sailing Tags
	FGameplayTag State_Movement_Sailing;
	FGameplayTag State_Movement_Sailing_Cruising;
	FGameplayTag State_Movement_Sailing_Anchored;
	FGameplayTag State_Vehicle_Watercraft;

	// Aircraft & Flight Dynamics Tags
	FGameplayTag State_Movement_Flying;
	FGameplayTag State_Movement_Flying_Airborne;
	FGameplayTag State_Movement_Flying_Stalling;
	FGameplayTag State_Movement_Flying_VTOL;
	FGameplayTag State_Vehicle_Aircraft;

	// Spacecraft & 6-DOF Zero-G Dynamics Tags
	FGameplayTag State_Movement_Spaceflight;
	FGameplayTag State_Movement_Spaceflight_Cruising;
	FGameplayTag State_Movement_Spaceflight_FlightAssistOff;
	FGameplayTag State_Movement_Spaceflight_Reentry;
	FGameplayTag State_Vehicle_Spacecraft;

	// Mech & Exosuit Locomotion Tags
	FGameplayTag State_Movement_Mech;
	FGameplayTag State_Movement_Mech_Walking;
	FGameplayTag State_Movement_Mech_JumpJets;
	FGameplayTag State_Movement_Mech_Overheated;
	FGameplayTag State_Vehicle_Mech;

	// Heavy Machinery & Hydraulic Physics Tags
	FGameplayTag State_Movement_Machinery;
	FGameplayTag State_Movement_Machinery_Operating;
	FGameplayTag State_Movement_Machinery_Lifting;
	FGameplayTag State_Movement_Machinery_Excavating;
	FGameplayTag State_Vehicle_Machinery;

	// Modular Structural Integrity & Collapse Tags
	FGameplayTag State_Building_Anchor;
	FGameplayTag State_Building_Supported;
	FGameplayTag State_Building_Stressed;
	FGameplayTag State_Building_Collapsing;

	// Power Grid & Electric Circuit Tags
	FGameplayTag State_Power_Powered;
	FGameplayTag State_Power_Unpowered;
	FGameplayTag State_Power_Overloaded;
	FGameplayTag State_Power_Charging;
	FGameplayTag State_Power_Discharging;

	// Pipe Networks, Fluids & Gas Mechanics Tags
	FGameplayTag State_Fluid_Flowing;
	FGameplayTag State_Fluid_Blocked;
	FGameplayTag State_Fluid_Pressurized;
	FGameplayTag State_Fluid_Leaking;
	FGameplayTag State_Fluid_Ruptured;

	// Conveyor Belts & Factory Logistics Tags
	FGameplayTag State_Logistics_Conveying;
	FGameplayTag State_Logistics_Jammed;
	FGameplayTag State_Logistics_Sorting;
	FGameplayTag State_Logistics_Merging;

	// Industrial Processing & Machinery Tags
	FGameplayTag State_Industrial_Processing;
	FGameplayTag State_Industrial_Idle;
	FGameplayTag State_Industrial_MissingIngredients;
	FGameplayTag State_Industrial_NoPower;
	FGameplayTag State_Industrial_OutputFull;
	FGameplayTag State_Industrial_Overheated;

	// Resource Extractors, Mining Drills & Wells Tags
	FGameplayTag State_Extractor_Drilling;
	FGameplayTag State_Extractor_Idle;
	FGameplayTag State_Extractor_Depleted;
	FGameplayTag State_Extractor_NoPower;
	FGameplayTag State_Extractor_OutputBlocked;
	FGameplayTag State_Extractor_Overheated;

	// Automated Freight Trains & Railroad Logistics Tags
	FGameplayTag State_Rail_Traveling;
	FGameplayTag State_Rail_Loading;
	FGameplayTag State_Rail_Unloading;
	FGameplayTag State_Rail_WaitingSignal;
	FGameplayTag State_Rail_Derailed;

	// Programmable Logic Controllers (PLC) & Circuit Networks Tags
	FGameplayTag State_Logic_Evaluating;
	FGameplayTag State_Logic_ConditionMet;
	FGameplayTag State_Logic_ConditionFailed;
	FGameplayTag State_Logic_Pulsing;
	FGameplayTag State_Logic_Disabled;

	// Industrial Cargo Drones & Sky Corridors Tags
	FGameplayTag State_Drone_Idle;
	FGameplayTag State_Drone_TakingOff;
	FGameplayTag State_Drone_InFlight;
	FGameplayTag State_Drone_Landing;
	FGameplayTag State_Drone_Recharging;
	FGameplayTag State_Drone_LowBattery;

	// Modular Space Elevator & Planetary Logistics Tags
	FGameplayTag State_SpaceElevator_Idle;
	FGameplayTag State_SpaceElevator_Ascending;
	FGameplayTag State_SpaceElevator_Descending;
	FGameplayTag State_SpaceElevator_PhaseCompleted;
	FGameplayTag State_SpaceElevator_Delivering;

	// Thermal Dynamics, Body Temperature & Hypothermia Tags
	FGameplayTag State_Thermal_Comfortable;
	FGameplayTag State_Thermal_Cold;
	FGameplayTag State_Thermal_Freezing;
	FGameplayTag State_Thermal_Warm;
	FGameplayTag State_Thermal_Overheating;
	FGameplayTag State_Thermal_Hypothermia;
	FGameplayTag State_Thermal_Heatstroke;

	// Advanced Metabolic Nutrition & Deficiency Tags
	FGameplayTag State_Metabolism_WellFed;
	FGameplayTag State_Metabolism_Hungry;
	FGameplayTag State_Metabolism_Starving;
	FGameplayTag State_Metabolism_Hydrated;
	FGameplayTag State_Metabolism_Thirsty;
	FGameplayTag State_Metabolism_Dehydrated;
	FGameplayTag State_Metabolism_Deficiency_VitaminC;
	FGameplayTag State_Metabolism_Deficiency_VitaminA;
	FGameplayTag State_Metabolism_Deficiency_Electrolytes;

	// Advanced Pathogen, Infection & Immune System Tags
	FGameplayTag State_Immunity_Infected;
	FGameplayTag State_Immunity_Incubating;
	FGameplayTag State_Immunity_Fever;
	FGameplayTag State_Immunity_Symptomatic;
	FGameplayTag State_Immunity_Recovering;
	FGameplayTag State_Immunity_Immune;

	// Advanced Physical Trauma, Hemorrhage & Fractures Tags
	FGameplayTag State_Trauma_Bleeding;
	FGameplayTag State_Trauma_ArterialBleed;
	FGameplayTag State_Trauma_Fracture_Arm;
	FGameplayTag State_Trauma_Fracture_Leg;
	FGameplayTag State_Trauma_TourniquetApplied;
	FGameplayTag State_Trauma_HypovolemicShock;

	// Advanced Dynamic Flora & Agriculture Tags
	FGameplayTag State_Crop_Seeded;
	FGameplayTag State_Crop_Sprouting;
	FGameplayTag State_Crop_Growing;
	FGameplayTag State_Crop_Harvestable;
	FGameplayTag State_Crop_Withered;
	FGameplayTag State_Crop_Fertilized;

	// Advanced Fauna Ecosystem, Domestication & Genetics Tags
	FGameplayTag State_Fauna_Wild;
	FGameplayTag State_Fauna_Taming;
	FGameplayTag State_Fauna_Domesticated;
	FGameplayTag State_Fauna_Pregnant;
	FGameplayTag State_Fauna_Juvenile;
	FGameplayTag State_Fauna_Mountable;

	// Advanced Planetary Atmosphere & Toxic Gas Hazard Tags
	FGameplayTag State_Atmosphere_Hazardous;
	FGameplayTag State_Atmosphere_Hypoxia;
	FGameplayTag State_Atmosphere_ToxicInhalation;
	FGameplayTag State_Atmosphere_SuitPressurized;
	FGameplayTag State_Atmosphere_FilterExhausted;
	FGameplayTag State_Atmosphere_Decompression;

	// Advanced Nuclear Radiation & Dosimetry Tags
	FGameplayTag State_Radiation_Exposed;
	FGameplayTag State_Radiation_LowDose;
	FGameplayTag State_Radiation_AcuteSickness;
	FGameplayTag State_Radiation_CriticalARS;
	FGameplayTag State_Radiation_LeadShielded;
	FGameplayTag State_Radiation_GeigerClicking;

	// Advanced Afflictions & Bio-Compounds Tags
	FGameplayTag State_Affliction_Impaired;
	FGameplayTag State_Affliction_Degradation;
	FGameplayTag State_Affliction_Paralyzed;
	FGameplayTag State_Affliction_Inoculated;
	FGameplayTag State_Affliction_Neutralized;

	// Advanced Surgery, Prosthetics & Organ Transplants Tags
	FGameplayTag State_Surgery_UnderAnesthesia;
	FGameplayTag State_Surgery_Operating;
	FGameplayTag State_Surgery_ProstheticInstalled;
	FGameplayTag State_Surgery_OrganRejection;
	FGameplayTag State_Surgery_CyberneticAugmented;

	// Dynamic Tick Throttling & Hierarchical LOD Tags
	FGameplayTag State_Throttling_LOD0;
	FGameplayTag State_Throttling_LOD1;
	FGameplayTag State_Throttling_LOD2;
	FGameplayTag State_Throttling_Background;
	FGameplayTag State_Throttling_Suspended;

	// Memory Layout & Cache Locality Tags
	FGameplayTag State_Memory_Optimized;
	FGameplayTag State_Memory_Contiguous;
	FGameplayTag State_Memory_ZeroAllocActive;

	// Multi-Threading & Async Task Graph Tags
	FGameplayTag State_Async_TaskRunning;
	FGameplayTag State_Async_DoubleBufferActive;
	FGameplayTag State_Async_WorkCompleted;

	// Hierarchical Spatial Partitioning & Grid Tags
	FGameplayTag State_Spatial_Indexed;
	FGameplayTag State_Spatial_CellActive;
	FGameplayTag State_Spatial_Queried;

	// Lock-Free Concurrency & Ring Buffer Tags
	FGameplayTag State_LockFree_Active;
	FGameplayTag State_LockFree_Buffering;
	FGameplayTag State_LockFree_Drained;

	// Event Bus Centralized Pub/Sub Tags
	FGameplayTag Event_Combat_DamageDealt;
	FGameplayTag Event_Inventory_ItemCrafted;
	FGameplayTag Event_Survival_StatusChanged;

	// Async Serialization & Binary Persistence Tags
	FGameplayTag State_Save_AsyncSaving;
	FGameplayTag State_Save_AsyncLoading;
	FGameplayTag State_Save_Serialized;

	// Dynamic Fault Tolerance & Fallback Tags
	FGameplayTag State_Fault_Degraded;
	FGameplayTag State_Fault_FallbackActive;
	FGameplayTag State_Fault_Resilient;

	// State Machine & Tag Matrix Verification Tags
	FGameplayTag State_Matrix_Verified;
	FGameplayTag State_Matrix_ConflictDetected;
	FGameplayTag State_Matrix_StrictEnforcement;

	// Live Reflection & Config Hot-Reloading Tags
	FGameplayTag State_Config_HotReloadActive;
	FGameplayTag State_Config_SchemaSynced;
	FGameplayTag State_Config_Observing;

	// In-Editor Visual Debugger & Viewport Overlay Tags
	FGameplayTag State_Debug_OverlayActive;
	FGameplayTag State_Debug_VisualizingPower;
	FGameplayTag State_Debug_VisualizingLogistics;

	// Real-Time Performance Profiler Tags
	FGameplayTag State_Profiler_Instrumented;
	FGameplayTag State_Profiler_BudgetExceeded;
	FGameplayTag State_Profiler_SamplingActive;

	// Procedural World Validation & Integrity Tags
	FGameplayTag State_Integrity_Audited;
	FGameplayTag State_Integrity_IssueDetected;
	FGameplayTag State_Integrity_Clean;

	// Automated Stress-Testing & Bot Swarm Tags
	FGameplayTag State_Stress_BotActive;
	FGameplayTag State_Stress_SimulatingAction;
	FGameplayTag State_Stress_SwarmMember;

	// Smart Object Activity Tags
	FGameplayTag Activity_TestInteraction;

private:
	static FSBGameplayTags Instance;
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);
};
