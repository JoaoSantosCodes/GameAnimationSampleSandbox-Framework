#include "Items/SBItemInstance.h"
#include "Items/SBItemDefinition.h"
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
}

const USBItemFragment* USBItemInstance::FindFragmentByClass(TSubclassOf<USBItemFragment> FragmentClass) const
{
	return ItemDef ? ItemDef->FindFragmentByClass(FragmentClass) : nullptr;
}
