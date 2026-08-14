#include "Subsystems/SBEventSubsystem.h"
#include "Utilities/SBLogCategories.h"

USBEventSubsystem::USBEventSubsystem()
{
}

void USBEventSubsystem::PublishEvent(FGameplayTag EventTag, UObject* Payload)
{
	struct FSBExecutionItem
	{
		uint8 Priority;
		TFunction<void()> Callback;

		bool operator<(const FSBExecutionItem& Other) const
		{
			return Priority < Other.Priority;
		}
	};

	TArray<FSBExecutionItem> ExecutionList;

	// Gather native listeners
	if (TArray<FSBNativeListener>* NativeList = NativeListeners.Find(EventTag))
	{
		// Clean up invalid delegates while gathering
		for (int32 i = NativeList->Num() - 1; i >= 0; --i)
		{
			const FSBNativeListener& Listener = (*NativeList)[i];
			if (Listener.Delegate.IsBound())
			{
				FSBExecutionItem Item;
				Item.Priority = static_cast<uint8>(Listener.Priority);
				Item.Callback = [Listener, EventTag, Payload]()
				{
					Listener.Delegate.ExecuteIfBound(EventTag, Payload);
				};
				ExecutionList.Add(Item);
			}
			else
			{
				NativeList->RemoveAt(i);
			}
		}
	}

	// Gather blueprint listeners
	if (FSBBlueprintListenerArray* BPListWrapper = BlueprintListeners.Find(EventTag))
	{
		TArray<FSBBlueprintListener>& BPList = BPListWrapper->Listeners;
		for (int32 i = BPList.Num() - 1; i >= 0; --i)
		{
			const FSBBlueprintListener& Listener = BPList[i];
			if (Listener.Delegate.IsBound())
			{
				FSBExecutionItem Item;
				Item.Priority = static_cast<uint8>(Listener.Priority);
				Item.Callback = [Listener, EventTag, Payload]()
				{
					Listener.Delegate.ExecuteIfBound(EventTag, Payload);
				};
				ExecutionList.Add(Item);
			}
			else
			{
				BPList.RemoveAt(i);
			}
		}
	}

	// Sort by priority ascending (High priority = 0, Lowest = 30)
	ExecutionList.StableSort();

	// Dispatch
	for (const FSBExecutionItem& Item : ExecutionList)
	{
		Item.Callback();
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
