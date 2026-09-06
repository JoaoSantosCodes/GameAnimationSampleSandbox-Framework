#include "Components/SBPersistenceComponent.h"
#include "GameFramework/Actor.h"

USBPersistenceComponent::USBPersistenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR
void USBPersistenceComponent::PostInitProperties()
{
	Super::PostInitProperties();
	AutoGenerateGuid();
}

void USBPersistenceComponent::PostEditImport()
{
	Super::PostEditImport();
	PersistentId.Reset();
	AutoGenerateGuid();
}

void USBPersistenceComponent::AutoGenerateGuid()
{
	AActor* Owner = GetOwner();
	if (Owner && !Owner->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		if (!PersistentId.IsValid())
		{
			PersistentId.Generate();
			Owner->MarkPackageDirty();
		}
	}
}
#endif
