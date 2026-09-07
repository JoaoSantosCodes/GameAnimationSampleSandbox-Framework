// Copyright 2026 João Santos. All Rights Reserved.
#include "Items/SBItemInstance.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment_Durability.h"
#include "Net/UnrealNetwork.h"

USBItemInstance::USBItemInstance()
{
}

void USBItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBItemInstance, ItemDef);
	DOREPLIFETIME(USBItemInstance, StackCount);
	DOREPLIFETIME(USBItemInstance, DynamicTags);
	DOREPLIFETIME(USBItemInstance, Durability);
	DOREPLIFETIME(USBItemInstance, UpgradeLevel);
}

void USBItemInstance::SetDurability_Implementation(float NewDurability)
{
	if (const USBItemFragment_Durability* DurabilityFragment = Cast<USBItemFragment_Durability>(FindFragmentByClass(USBItemFragment_Durability::StaticClass())))
	{
		Durability = FMath::Clamp(NewDurability, 0.0f, DurabilityFragment->MaxDurability);
	}
	else
	{
		Durability = NewDurability;
	}
}

void USBItemInstance::ConsumeDurability_Implementation(float Amount)
{
	if (Amount <= 0.0f) return;
	
	float NewDurability = Durability - Amount;
	SetDurability_Implementation(NewDurability);
}

const USBItemFragment* USBItemInstance::FindFragmentByClass(TSubclassOf<USBItemFragment> FragmentClass) const
{
	return ItemDef ? ItemDef->FindFragmentByClass(FragmentClass) : nullptr;
}
