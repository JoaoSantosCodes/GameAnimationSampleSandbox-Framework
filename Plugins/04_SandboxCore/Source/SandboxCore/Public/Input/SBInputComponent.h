#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Input/SBInputConfig.h"
#include "SBInputComponent.generated.h"

UCLASS()
class SANDBOXCORE_API USBInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template<class UserClass, typename FuncType>
	void BindActionByTag(const USBInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
	{
		if (!InputConfig) return;

		if (const UInputAction* Action = InputConfig->FindInputActionForTag(InputTag))
		{
			if (Func)
			{
				BindAction(Action, TriggerEvent, Object, Func, InputTag);
			}
		}
	}
};
