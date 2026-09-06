#include "Components/SBStressTestBotComponent.h"
#include "Subsystems/SBStressTestSubsystem.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBStressTestBotComponent::USBStressTestBotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBStressTestBotComponent::OnInitialize_Implementation()
{
	SyncTags();
}

void USBStressTestBotComponent::OnReady_Implementation()
{
	SyncTags();
}

void USBStressTestBotComponent::OnShutdown_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Stress_BotActive);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Stress_SwarmMember);
			ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Stress_SimulatingAction);
		}
	}
}

void USBStressTestBotComponent::ExecuteSimulatedAction(ESBBotSimAction Action, bool bSuccess)
{
	CurrentSimAction = Action;
	if (UWorld* World = GetWorld())
	{
		if (USBStressTestSubsystem* Subsystem = World->GetSubsystem<USBStressTestSubsystem>())
		{
			Subsystem->RecordBotAction(BotId, Action, bSuccess);
		}
	}
	SyncTags();
}

void USBStressTestBotComponent::SyncTags()
{
	if (AActor* Owner = GetOwner())
	{
		if (UActorComponent* StateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass()))
		{
			const FSBGameplayTags& Tags = FSBGameplayTags::Get();
			ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Stress_BotActive);
			ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Stress_SwarmMember);
			if (CurrentSimAction != ESBBotSimAction::Idle)
			{
				ISBStateComponentInterface::Execute_AddTag(StateComp, Tags.State_Stress_SimulatingAction);
			}
			else
			{
				ISBStateComponentInterface::Execute_RemoveTag(StateComp, Tags.State_Stress_SimulatingAction);
			}
		}
	}
}
