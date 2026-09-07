// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Subsystems/SBStressTestSubsystem.h"
#include "Components/SBStressTestBotComponent.h"
#include "SBCoreTestTypes.h"
#include "SBGameplayTags.h"
#include "GameFramework/Actor.h"

BEGIN_DEFINE_SPEC(FSBStressTestTestsSpec, "Sandbox.Core.StressTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	UWorld* TestWorld = nullptr;
	USBStressTestSubsystem* StressSubsystem = nullptr;
	AActor* TestActor = nullptr;
	USBCoreTestStateComponent* StateComp = nullptr;
	USBStressTestBotComponent* BotComp = nullptr;
END_DEFINE_SPEC(FSBStressTestTestsSpec)

void FSBStressTestTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("StressTestWorld"));
		if (TestWorld)
		{
			StressSubsystem = TestWorld->GetSubsystem<USBStressTestSubsystem>();
			if (StressSubsystem)
			{
				StressSubsystem->ResetSubsystem();
			}

			TestActor = TestWorld->SpawnActor<AActor>();
			if (TestActor)
			{
				StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
				TestActor->AddInstanceComponent(StateComp);
				StateComp->RegisterComponent();

				BotComp = NewObject<USBStressTestBotComponent>(TestActor, TEXT("BotComp"));
				TestActor->AddInstanceComponent(BotComp);
				BotComp->RegisterComponent();
			}
		}
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
			StressSubsystem = nullptr;
			TestActor = nullptr;
			StateComp = nullptr;
			BotComp = nullptr;
		}
	});

	It("Should spawn bot swarm and initialize agent states", [this]()
	{
		TestNotNull("StressSubsystem valid", StressSubsystem);

		int32 Spawned = StressSubsystem->SpawnBotSwarm(10);
		TestEqual("Spawned 10 bots", Spawned, 10);

		FSBStressTestMetrics Metrics = StressSubsystem->GetMetrics();
		TestEqual("Total simulated bots tracked in metrics", Metrics.TotalSimulatedBots, 10);

		FSBBotSimAgentState Bot0 = StressSubsystem->GetBotState(0);
		TestTrue("Bot 0 is active", Bot0.bIsActive);
		TestEqual("Bot 0 starts at Idle action", (int32)Bot0.CurrentAction, (int32)ESBBotSimAction::Idle);
		TestEqual("Bot 0 actions completed starts at 0", Bot0.ActionsCompleted, 0);
	});

	It("Should execute stress tick cycles and advance bot actions deterministically", [this]()
	{
		TestNotNull("StressSubsystem valid", StressSubsystem);

		StressSubsystem->SpawnBotSwarm(5);
		StressSubsystem->ExecuteStressTick(0.1f, 3);

		FSBStressTestMetrics Metrics = StressSubsystem->GetMetrics();
		TestEqual("Total actions executed across 5 bots * 3 cycles = 15", Metrics.TotalActionsExecuted, 15);

		FSBBotSimAgentState Bot0 = StressSubsystem->GetBotState(0);
		TestEqual("Bot 0 completed 3 actions", Bot0.ActionsCompleted, 3);
		TestEqual("Bot 0 advanced to VehicleMount (Action 3)", (int32)Bot0.CurrentAction, (int32)ESBBotSimAction::VehicleMount);
	});

	It("Should track stress test metrics and record zero deadlocks under workload", [this]()
	{
		TestNotNull("StressSubsystem valid", StressSubsystem);

		StressSubsystem->RecordBotAction(101, ESBBotSimAction::CombatMelee, true);
		StressSubsystem->RecordBotAction(102, ESBBotSimAction::HarvestMining, true);
		StressSubsystem->RecordBotAction(103, ESBBotSimAction::SurgeryProcedure, true);

		FSBStressTestMetrics Metrics = StressSubsystem->GetMetrics();
		TestEqual("Total actions executed is 3", Metrics.TotalActionsExecuted, 3);
		TestEqual("Total deadlocks is 0", Metrics.TotalDeadlocksDetected, 0);
		TestEqual("Total unhandled exceptions is 0", Metrics.TotalUnhandledExceptions, 0);
	});

	It("Should auto-subscribe stress test bot component, simulate actions, update tags, and cleanup", [this]()
	{
		TestNotNull("BotComp valid", BotComp);
		TestNotNull("StateComp valid", StateComp);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		BotComp->OnInitialize_Implementation();
		TestTrue("BotActive tag present", StateComp->HasTag(Tags.State_Stress_BotActive));
		TestTrue("SwarmMember tag present", StateComp->HasTag(Tags.State_Stress_SwarmMember));
		TestFalse("SimulatingAction tag not present on Idle", StateComp->HasTag(Tags.State_Stress_SimulatingAction));

		// Executa ação de combate
		BotComp->ExecuteSimulatedAction(ESBBotSimAction::CombatMelee);
		TestTrue("SimulatingAction tag present during CombatMelee", StateComp->HasTag(Tags.State_Stress_SimulatingAction));
		TestEqual("Current action is CombatMelee", (int32)BotComp->GetCurrentAction(), (int32)ESBBotSimAction::CombatMelee);

		// Retorna para Idle
		BotComp->ExecuteSimulatedAction(ESBBotSimAction::Idle);
		TestFalse("SimulatingAction tag cleared on Idle", StateComp->HasTag(Tags.State_Stress_SimulatingAction));

		// Shutdown
		BotComp->OnShutdown_Implementation();
		TestFalse("BotActive tag removed on shutdown", StateComp->HasTag(Tags.State_Stress_BotActive));
		TestFalse("SwarmMember tag removed on shutdown", StateComp->HasTag(Tags.State_Stress_SwarmMember));
		TestFalse("SimulatingAction tag removed on shutdown", StateComp->HasTag(Tags.State_Stress_SimulatingAction));
	});
}
