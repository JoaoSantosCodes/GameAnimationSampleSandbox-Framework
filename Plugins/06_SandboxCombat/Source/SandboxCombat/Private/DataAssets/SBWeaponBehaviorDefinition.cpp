#include "DataAssets/SBWeaponBehaviorDefinition.h"

USBWeaponBehaviorDefinition::USBWeaponBehaviorDefinition()
{
	ActiveSocketName = FName(TEXT("hand_rSocket"));
	HolsterSocketName = FName(TEXT("spine_03Socket"));
	CriticalDamageMultiplier = 2.0f;
	CriticalBoneNames.Add(FName(TEXT("head")));
	CriticalBoneNames.Add(FName(TEXT("neck_01")));
	CriticalBoneNames.Add(FName(TEXT("neck_02")));
}
