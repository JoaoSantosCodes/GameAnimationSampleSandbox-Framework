#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment.h"

const USBItemFragment* USBItemDefinition::FindFragmentByClass(TSubclassOf<USBItemFragment> FragmentClass) const
{
	if (FragmentClass)
	{
		for (const TObjectPtr<USBItemFragment>& Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}
	return nullptr;
}
