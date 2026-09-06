#include "Subsystems/SBEventSubsystem.h"
#include "Utilities/SBLogCategories.h"

USBEventSubsystem::USBEventSubsystem()
{
}

void USBEventSubsystem::PublishEvent(FGameplayTag EventTag, UObject* Payload)
{
	static const ESBEventPriority PriorityTiers[] = {
		ESBEventPriority::High,
		ESBEventPriority::Medium,
		ESBEventPriority::Low,
		ESBEventPriority::Lowest
	};

	for (ESBEventPriority CurrentPriority : PriorityTiers)
	{
		// Process native listeners in this priority tier
		if (TArray<FSBNativeListener>* NativeList = NativeListeners.Find(EventTag))
		{
			for (int32 i = NativeList->Num() - 1; i >= 0; --i)
			{
				FSBNativeListener& Listener = (*NativeList)[i];
				if (Listener.Priority == CurrentPriority)
				{
					if (Listener.Delegate.IsBound())
					{
						Listener.Delegate.Execute(EventTag, Payload);
					}
					else
					{
						NativeList->RemoveAt(i);
					}
				}
			}
		}

		// Process blueprint listeners in this priority tier
		if (FSBBlueprintListenerArray* BPListWrapper = BlueprintListeners.Find(EventTag))
		{
			TArray<FSBBlueprintListener>& BPList = BPListWrapper->Listeners;
			for (int32 i = BPList.Num() - 1; i >= 0; --i)
			{
				FSBBlueprintListener& Listener = BPList[i];
				if (Listener.Priority == CurrentPriority)
				{
					if (Listener.Delegate.IsBound())
					{
						Listener.Delegate.Execute(EventTag, Payload);
					}
					else
					{
						BPList.RemoveAt(i);
					}
				}
			}
		}
	}
}

FDelegateHandle USBEventSubsystem::SubscribeToEventNative(FGameplayTag EventTag, ESBEventPriority Priority, FSBNativeEventDelegate Delegate)
{
	if (!Delegate.IsBound())
	{
		return FDelegateHandle();
	}

	FDelegateHandle Handle(FDelegateHandle::GenerateNewHandle);
	FSBNativeListener Listener;
	Listener.Priority = Priority;
	Listener.Delegate = Delegate;
	Listener.Handle = Handle;

	NativeListeners.FindOrAdd(EventTag).Add(Listener);
	return Handle;
}

void USBEventSubsystem::UnsubscribeFromEventNative(FGameplayTag EventTag, FDelegateHandle Handle)
{
	if (TArray<FSBNativeListener>* List = NativeListeners.Find(EventTag))
	{
		for (int32 i = 0; i < List->Num(); ++i)
		{
			if ((*List)[i].Handle == Handle)
			{
				List->RemoveAt(i);
				break;
			}
		}
	}
}

void USBEventSubsystem::SubscribeToEvent(FGameplayTag EventTag, ESBEventPriority Priority, FSBBlueprintEventDelegate Delegate)
{
	if (!Delegate.IsBound())
	{
		return;
	}

	FSBBlueprintListenerArray& ListWrapper = BlueprintListeners.FindOrAdd(EventTag);
	for (const FSBBlueprintListener& Existing : ListWrapper.Listeners)
	{
		if (Existing.Delegate == Delegate)
		{
			return; // Evita registro duplicado
		}
	}

	FSBBlueprintListener Listener;
	Listener.Priority = Priority;
	Listener.Delegate = Delegate;

	ListWrapper.Listeners.Add(Listener);
}

void USBEventSubsystem::UnsubscribeFromEvent(FGameplayTag EventTag, FSBBlueprintEventDelegate Delegate)
{
	if (FSBBlueprintListenerArray* ListWrapper = BlueprintListeners.Find(EventTag))
	{
		TArray<FSBBlueprintListener>& List = ListWrapper->Listeners;
		for (int32 i = 0; i < List.Num(); ++i)
		{
			if (List[i].Delegate == Delegate)
			{
				List.RemoveAt(i);
				break;
			}
		}
	}
}
