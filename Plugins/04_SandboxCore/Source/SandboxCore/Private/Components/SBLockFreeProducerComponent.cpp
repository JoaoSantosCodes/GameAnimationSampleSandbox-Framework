#include "Components/SBLockFreeProducerComponent.h"
#include "Subsystems/SBLockFreeEventSubsystem.h"
#include "SBGameplayTags.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBLockFreeProducerComponent::USBLockFreeProducerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBLockFreeProducerComponent::OnInitialize_Implementation()
{
	SyncTags();
}

void USBLockFreeProducerComponent::OnReady_Implementation()
{
	SyncTags();
}

void USBLockFreeProducerComponent::OnShutdown_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_LockFree_Active);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_LockFree_Buffering);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_LockFree_Drained);
		}
	}
}

bool USBLockFreeProducerComponent::ProduceEvent(int32 EventType, float PayloadFloat)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	USBLockFreeEventSubsystem* Subsystem = World->GetSubsystem<USBLockFreeEventSubsystem>();
	if (!Subsystem)
	{
		return false;
	}

	int32 EventID = NextEventID++;
	FSBLockFreeEvent NewEvent(EventID, EntityID, EventType, PayloadFloat, FDateTime::UtcNow().GetTicks());
	bool bSuccess = Subsystem->EnqueueEvent(NewEvent);

	if (bSuccess)
	{
		ProducedCount++;
		SyncTags();
		OnEventProduced.Broadcast(EventID, EventType, PayloadFloat);
	}

	return bSuccess;
}

void USBLockFreeProducerComponent::SyncTags()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	if (!StateComp)
	{
		return;
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_LockFree_Active);

	if (ProducedCount > 0)
	{
		ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_LockFree_Buffering);
	}
}
