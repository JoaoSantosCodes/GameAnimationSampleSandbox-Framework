#include "Components/SBVisualDebugOverlayComponent.h"
#include "Subsystems/SBVisualDebuggerSubsystem.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBVisualDebugOverlayComponent::USBVisualDebugOverlayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBVisualDebugOverlayComponent::OnInitialize_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (USBVisualDebuggerSubsystem* Subsystem = World->GetSubsystem<USBVisualDebuggerSubsystem>())
		{
			Subsystem->OnOverlayCategoryToggled.AddDynamic(this, &USBVisualDebugOverlayComponent::HandleCategoryToggled);
		}
	}
	SyncTags();
}

void USBVisualDebugOverlayComponent::OnReady_Implementation()
{
	SyncTags();
}

void USBVisualDebugOverlayComponent::OnShutdown_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (USBVisualDebuggerSubsystem* Subsystem = World->GetSubsystem<USBVisualDebuggerSubsystem>())
		{
			Subsystem->OnOverlayCategoryToggled.RemoveDynamic(this, &USBVisualDebugOverlayComponent::HandleCategoryToggled);
		}
	}

	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Debug_OverlayActive);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Debug_VisualizingPower);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Debug_VisualizingLogistics);
		}
	}
}

void USBVisualDebugOverlayComponent::PushDebugLine(const FVector& TargetLocation, const FColor& Color, const FString& Label)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	USBVisualDebuggerSubsystem* Subsystem = World->GetSubsystem<USBVisualDebuggerSubsystem>();
	if (!Subsystem || !Subsystem->IsCategoryEnabled(ComponentCategory))
	{
		return;
	}

	FSBOverlayRenderItem Item(Owner->GetActorLocation(), TargetLocation, Color, Label, ComponentCategory);
	Subsystem->QueueRenderItem(Item);
	PushedLinesCount++;
}

void USBVisualDebugOverlayComponent::HandleCategoryToggled(ESBOverlayCategory Category, bool bEnabled)
{
	if (Category == ComponentCategory || Category == ESBOverlayCategory::All)
	{
		SyncTags();
	}
}

void USBVisualDebugOverlayComponent::SyncTags()
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
	UWorld* World = GetWorld();
	if (World)
	{
		if (USBVisualDebuggerSubsystem* Subsystem = World->GetSubsystem<USBVisualDebuggerSubsystem>())
		{
			if (Subsystem->IsCategoryEnabled(ComponentCategory))
			{
				ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Debug_OverlayActive);
				if (ComponentCategory == ESBOverlayCategory::PowerGrid)
				{
					ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Debug_VisualizingPower);
				}
				else if (ComponentCategory == ESBOverlayCategory::ConveyorNetwork || ComponentCategory == ESBOverlayCategory::DroneRoutes)
				{
					ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Debug_VisualizingLogistics);
				}
				return;
			}
		}
	}

	ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Debug_OverlayActive);
	ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Debug_VisualizingPower);
	ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Debug_VisualizingLogistics);
}
