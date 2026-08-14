#include "Components/SBComponentFactory.h"
#include "DataAssets/SBComponentSetDataAsset.h"
#include "Interfaces/SBComponentInterface.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Utilities/SBLogCategories.h"

struct FSortHelper
{
	const TArray<FSBComponentSetEntry>& RawEntries;
	TArray<FSBComponentSetEntry>& SortedEntries;
	TSet<TSubclassOf<UActorComponent>>& VisitedClasses;
	TSet<TSubclassOf<UActorComponent>>& ProcessingClasses;

	void Sort(const FSBComponentSetEntry& Entry)
	{
		if (VisitedClasses.Contains(Entry.ComponentClass))
		{
			return;
		}
		if (ProcessingClasses.Contains(Entry.ComponentClass))
		{
			UE_LOG(LogSandboxCore, Error, TEXT("Circular dependency detected for component class %s!"), *Entry.ComponentClass->GetName());
			return;
		}

		ProcessingClasses.Add(Entry.ComponentClass);

		for (TSubclassOf<UActorComponent> DepClass : Entry.Dependencies)
		{
			if (DepClass)
			{
				const FSBComponentSetEntry* DepEntry = RawEntries.FindByPredicate([&](const FSBComponentSetEntry& E) {
					return E.ComponentClass && E.ComponentClass->IsChildOf(DepClass);
				});

				if (DepEntry)
				{
					Sort(*DepEntry);
				}
				else
				{
					UE_LOG(LogSandboxCore, Warning, TEXT("Component %s depends on %s, but it's not present in the ComponentSet!"), *Entry.ComponentClass->GetName(), *DepClass->GetName());
				}
			}
		}

		ProcessingClasses.Remove(Entry.ComponentClass);
		VisitedClasses.Add(Entry.ComponentClass);
		SortedEntries.Add(Entry);
	}
};

void USBComponentFactory::InitializeComponentsFromSet(AActor* TargetActor, USBComponentSetDataAsset* ComponentSet)
{
	if (!TargetActor || !ComponentSet)
	{
		return;
	}

	TArray<FSBComponentSetEntry> RawEntries = ComponentSet->Components;
	TArray<FSBComponentSetEntry> SortedEntries;
	TSet<TSubclassOf<UActorComponent>> VisitedClasses;
	TSet<TSubclassOf<UActorComponent>> ProcessingClasses;

	FSortHelper Sorter{ RawEntries, SortedEntries, VisitedClasses, ProcessingClasses };

	for (const FSBComponentSetEntry& Entry : RawEntries)
	{
		if (Entry.ComponentClass)
		{
			Sorter.Sort(Entry);
		}
	}

	UGameFrameworkComponentManager* ComponentManager = UGameFrameworkComponentManager::GetForActor(TargetActor);
	TArray<UActorComponent*> SpawnedComponents;

	for (const FSBComponentSetEntry& Entry : SortedEntries)
	{
		if (!Entry.ComponentClass) continue;

		if (TargetActor->GetComponentByClass(Entry.ComponentClass))
		{
			UE_LOG(LogSandboxCore, Warning, TEXT("Component of class %s already exists on actor %s! Skipping spawn."), *Entry.ComponentClass->GetName(), *TargetActor->GetName());
			continue;
		}

		UActorComponent* NewComp = NewObject<UActorComponent>(TargetActor, Entry.ComponentClass);
		if (NewComp)
		{
			NewComp->RegisterComponent();
			SpawnedComponents.Add(NewComp);

			if (ComponentManager)
			{
				ComponentManager->AddReceiver(TargetActor);
			}

			if (NewComp->GetClass()->ImplementsInterface(USBComponentInterface::StaticClass()))
			{
				ISBComponentInterface::Execute_OnComponentCreated(NewComp);
			}
		}
	}

	// Trigger Lifecycle Sequence
	for (UActorComponent* Comp : SpawnedComponents)
	{
		if (Comp->GetClass()->ImplementsInterface(USBComponentInterface::StaticClass()))
		{
			ISBComponentInterface::Execute_OnPreInitialize(Comp);
		}
	}

	for (UActorComponent* Comp : SpawnedComponents)
	{
		if (Comp->GetClass()->ImplementsInterface(USBComponentInterface::StaticClass()))
		{
			ISBComponentInterface::Execute_OnInitialize(Comp);
		}
	}

	for (UActorComponent* Comp : SpawnedComponents)
	{
		if (Comp->GetClass()->ImplementsInterface(USBComponentInterface::StaticClass()))
		{
			ISBComponentInterface::Execute_OnPostInitialize(Comp);
		}
	}

	for (UActorComponent* Comp : SpawnedComponents)
	{
		if (Comp->GetClass()->ImplementsInterface(USBComponentInterface::StaticClass()))
		{
			ISBComponentInterface::Execute_OnReady(Comp);
		}
	}
}
