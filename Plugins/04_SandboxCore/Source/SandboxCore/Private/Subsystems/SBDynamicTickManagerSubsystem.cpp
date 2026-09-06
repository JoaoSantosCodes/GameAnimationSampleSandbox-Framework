#include "Subsystems/SBDynamicTickManagerSubsystem.h"
#include "Components/SBDynamicTickThrottlingComponent.h"
#include "GameFramework/Actor.h"

void USBDynamicTickManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisteredComponents.Empty();
}

void USBDynamicTickManagerSubsystem::Deinitialize()
{
	RegisteredComponents.Empty();
	Super::Deinitialize();
}

void USBDynamicTickManagerSubsystem::RegisterThrottledComponent(USBDynamicTickThrottlingComponent* Comp)
{
	if (Comp && !RegisteredComponents.Contains(Comp))
	{
		RegisteredComponents.Add(Comp);
	}
}

void USBDynamicTickManagerSubsystem::UnregisterThrottledComponent(USBDynamicTickThrottlingComponent* Comp)
{
	if (Comp)
	{
		RegisteredComponents.Remove(Comp);
	}
}

void USBDynamicTickManagerSubsystem::UpdateAllLODs(const FVector& ViewerLocation)
{
	for (int32 Index = RegisteredComponents.Num() - 1; Index >= 0; --Index)
	{
		if (!RegisteredComponents[Index].IsValid())
		{
			RegisteredComponents.RemoveAt(Index);
			continue;
		}

		USBDynamicTickThrottlingComponent* Comp = RegisteredComponents[Index].Get();
		if (AActor* Owner = Comp->GetOwner())
		{
			float Distance = FVector::Dist(Owner->GetActorLocation(), ViewerLocation);
			Comp->UpdateDistanceToViewer(Distance, true);
		}
	}
}

int32 USBDynamicTickManagerSubsystem::GetRegisteredCount() const
{
	int32 ValidCount = 0;
	for (const auto& WeakComp : RegisteredComponents)
	{
		if (WeakComp.IsValid())
		{
			ValidCount++;
		}
	}
	return ValidCount;
}

int32 USBDynamicTickManagerSubsystem::GetCountByLOD(ESBTickLODLevel LOD) const
{
	int32 Count = 0;
	for (const auto& WeakComp : RegisteredComponents)
	{
		if (WeakComp.IsValid() && WeakComp->GetCurrentLOD() == LOD)
		{
			Count++;
		}
	}
	return Count;
}
