#include "Types/SBCommonTypes.h"

UObject* USBBehaviorRegistry::GetOrInstantiateBehavior(FGameplayTag BehaviorTag, TSubclassOf<UObject> BehaviorClass)
{
	if (!BehaviorTag.IsValid() || !BehaviorClass)
	{
		return nullptr;
	}

	if (TObjectPtr<UObject>* Found = InstantiatedBehaviors.Find(BehaviorTag))
	{
		return *Found;
	}

	UObject* NewBehavior = NewObject<UObject>(this, BehaviorClass);
	if (NewBehavior)
	{
		InstantiatedBehaviors.Add(BehaviorTag, NewBehavior);
		return NewBehavior;
	}

	return nullptr;
}
