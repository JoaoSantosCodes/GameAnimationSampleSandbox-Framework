#include "Components/SBLiveConfigObserverComponent.h"
#include "Subsystems/SBLiveConfigSubsystem.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBLiveConfigObserverComponent::USBLiveConfigObserverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBLiveConfigObserverComponent::OnInitialize_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (USBLiveConfigSubsystem* ConfigSubsystem = World->GetSubsystem<USBLiveConfigSubsystem>())
		{
			ConfigSubsystem->OnConfigSchemaHotReloaded.AddDynamic(this, &USBLiveConfigObserverComponent::HandleSchemaHotReloaded);
			ObservedVersion = ConfigSubsystem->GetSchemaVersion(WatchedSchema);
		}
	}
	SyncTags();
}

void USBLiveConfigObserverComponent::OnReady_Implementation()
{
	SyncTags();
}

void USBLiveConfigObserverComponent::OnShutdown_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (USBLiveConfigSubsystem* ConfigSubsystem = World->GetSubsystem<USBLiveConfigSubsystem>())
		{
			ConfigSubsystem->OnConfigSchemaHotReloaded.RemoveDynamic(this, &USBLiveConfigObserverComponent::HandleSchemaHotReloaded);
		}
	}

	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Config_Observing);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Config_SchemaSynced);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Config_HotReloadActive);
		}
	}
}

void USBLiveConfigObserverComponent::HandleSchemaHotReloaded(FName SchemaName, int32 NewVersion)
{
	if (SchemaName == WatchedSchema)
	{
		ObservedVersion = NewVersion;
		if (AActor* Owner = GetOwner())
		{
			if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
			{
				const FSBGameplayTags& Tags = FSBGameplayTags::Get();
				ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Config_HotReloadActive);
				ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Config_SchemaSynced);
			}
		}
		OnSchemaUpdated.Broadcast(SchemaName, NewVersion);
	}
}

void USBLiveConfigObserverComponent::SyncTags()
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
	ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Config_Observing);
	ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Config_SchemaSynced);
}
