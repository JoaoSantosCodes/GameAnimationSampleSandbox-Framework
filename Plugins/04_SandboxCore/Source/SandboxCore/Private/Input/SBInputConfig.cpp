#include "Input/SBInputConfig.h"

const UInputAction* USBInputConfig::FindInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FSBInputActionMapping& Mapping : InputActions)
	{
		if (Mapping.InputTag == InputTag)
		{
			return Mapping.InputAction;
		}
	}
	return nullptr;
}
