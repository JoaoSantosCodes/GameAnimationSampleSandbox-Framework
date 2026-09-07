// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBStressTestSubsystem.h"

USBStressTestSubsystem::USBStressTestSubsystem()
{
}

void USBStressTestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetSubsystem();
}

void USBStressTestSubsystem::Deinitialize()
{
	ResetSubsystem();
	Super::Deinitialize();
}

int32 USBStressTestSubsystem::SpawnBotSwarm(int32 BotCount)
{
	for (int32 i = 0; i < BotCount; ++i)
	{
		FSBBotSimAgentState Bot;
		Bot.BotIndex = i;
		Bot.CurrentAction = ESBBotSimAction::Idle;
		Bot.ActionsCompleted = 0;
		Bot.ErrorsEncountered = 0;
		Bot.bIsActive = true;
		SwarmBots.Add(i, Bot);
	}
	Metrics.TotalSimulatedBots = SwarmBots.Num();
	return SwarmBots.Num();
}

void USBStressTestSubsystem::ExecuteStressTick(float DeltaTime, int32 ActionCyclesPerBot)
{
	Metrics.StressDurationSeconds += DeltaTime;
	int32 ExecutedThisCycle = 0;

	for (auto& Pair : SwarmBots)
	{
		FSBBotSimAgentState& Bot = Pair.Value;
		if (!Bot.bIsActive)
		{
			continue;
		}

		for (int32 c = 0; c < ActionCyclesPerBot; ++c)
		{
			uint8 NextActionInt = (static_cast<uint8>(Bot.CurrentAction) + 1) % 6;
			Bot.CurrentAction = static_cast<ESBBotSimAction>(NextActionInt);
			Bot.ActionsCompleted++;
			Metrics.TotalActionsExecuted++;
			ExecutedThisCycle++;
		}
	}

	OnStressTestCycleCompleted.Broadcast(SwarmBots.Num(), ExecutedThisCycle);
}

void USBStressTestSubsystem::RecordBotAction(int32 BotIndex, ESBBotSimAction Action, bool bSuccess)
{
	if (!SwarmBots.Contains(BotIndex))
	{
		FSBBotSimAgentState NewBot;
		NewBot.BotIndex = BotIndex;
		NewBot.bIsActive = true;
		SwarmBots.Add(BotIndex, NewBot);
		Metrics.TotalSimulatedBots = SwarmBots.Num();
	}

	FSBBotSimAgentState* Bot = SwarmBots.Find(BotIndex);
	if (Bot)
	{
		Bot->CurrentAction = Action;
		Bot->ActionsCompleted++;
		if (!bSuccess)
		{
			Bot->ErrorsEncountered++;
			Metrics.TotalUnhandledExceptions++;
		}
		Metrics.TotalActionsExecuted++;
	}
}

FSBBotSimAgentState USBStressTestSubsystem::GetBotState(int32 BotIndex) const
{
	if (const FSBBotSimAgentState* Bot = SwarmBots.Find(BotIndex))
	{
		return *Bot;
	}
	return FSBBotSimAgentState();
}

void USBStressTestSubsystem::ResetSubsystem()
{
	SwarmBots.Empty();
	Metrics = FSBStressTestMetrics();
}
