// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Subsystems/SBPerformanceProfilerSubsystem.h"
#include "Components/SBPerformanceInstrumentComponent.h"
#include "SBCoreTestTypes.h"
#include "SBGameplayTags.h"
#include "GameFramework/Actor.h"

BEGIN_DEFINE_SPEC(FSBPerformanceProfilerTestsSpec, "Sandbox.Core.PerformanceProfiler", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	UWorld* TestWorld = nullptr;
	USBPerformanceProfilerSubsystem* ProfilerSubsystem = nullptr;
	AActor* TestActor = nullptr;
	USBCoreTestStateComponent* StateComp = nullptr;
	USBPerformanceInstrumentComponent* InstrumentComp = nullptr;
END_DEFINE_SPEC(FSBPerformanceProfilerTestsSpec)

void FSBPerformanceProfilerTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("ProfilerTestWorld"));
		if (TestWorld)
		{
			ProfilerSubsystem = TestWorld->GetSubsystem<USBPerformanceProfilerSubsystem>();
			if (ProfilerSubsystem)
			{
				ProfilerSubsystem->ResetSubsystem();
			}

			TestActor = TestWorld->SpawnActor<AActor>();
			if (TestActor)
			{
				StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
				TestActor->AddInstanceComponent(StateComp);
				StateComp->RegisterComponent();

				InstrumentComp = NewObject<USBPerformanceInstrumentComponent>(TestActor, TEXT("InstrumentComp"));
				TestActor->AddInstanceComponent(InstrumentComp);
				InstrumentComp->RegisterComponent();
			}
		}
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
			ProfilerSubsystem = nullptr;
			TestActor = nullptr;
			StateComp = nullptr;
			InstrumentComp = nullptr;
		}
	});

	It("Should record component sample and calculate statistical metrics (Min, Max, Avg, SampleCount)", [this]()
	{
		TestNotNull("ProfilerSubsystem valid", ProfilerSubsystem);

		const FName CompName = TEXT("CombatComponent");
		ProfilerSubsystem->RecordComponentSample(CompName, 50.0f, 1024);
		ProfilerSubsystem->RecordComponentSample(CompName, 150.0f, 2048);
		ProfilerSubsystem->RecordComponentSample(CompName, 100.0f, 2048);

		FSBComponentSampleData Stats = ProfilerSubsystem->GetComponentStats(CompName);
		TestEqual("SampleCount is 3", Stats.SampleCount, 3);
		TestEqual("MinDurationUs is 50.0", Stats.MinDurationUs, 50.0f);
		TestEqual("MaxDurationUs is 150.0", Stats.MaxDurationUs, 150.0f);
		TestEqual("AverageDurationUs is 100.0", Stats.AverageDurationUs, 100.0f);
		TestEqual("LastExecutionDurationUs is 100.0", Stats.LastExecutionDurationUs, 100.0f);
		TestEqual("EstimatedMemoryBytes is 2048", Stats.EstimatedMemoryBytes, static_cast<int64>(2048));
	});

	It("Should monitor budget threshold and broadcast OnComponentBudgetExceeded when overrun occurs", [this]()
	{
		TestNotNull("ProfilerSubsystem valid", ProfilerSubsystem);

		const FName CompName = TEXT("InventoryComponent");
		ProfilerSubsystem->SetBudgetThreshold(CompName, 100.0f);

		USBCoreTestBudgetListener* Listener = NewObject<USBCoreTestBudgetListener>(ProfilerSubsystem);
		ProfilerSubsystem->OnComponentBudgetExceeded.AddDynamic(Listener, &USBCoreTestBudgetListener::HandleBudgetExceeded);

		// Dentro do orçamento -> nenhum broadcast
		ProfilerSubsystem->RecordComponentSample(CompName, 80.0f, 512);
		TestFalse("Budget not exceeded for 80us", Listener->bFired);

		// Estoura orçamento -> dispara delegate
		ProfilerSubsystem->RecordComponentSample(CompName, 130.0f, 512);
		TestTrue("Budget exceeded triggered for 130us", Listener->bFired);
		TestEqual("Captured duration matches", Listener->CapturedDurationUs, 130.0f);
	});

	It("Should aggregate global subsystem metrics and identify hot components exceeding threshold", [this]()
	{
		TestNotNull("ProfilerSubsystem valid", ProfilerSubsystem);

		ProfilerSubsystem->SetBudgetThreshold(TEXT("CompA"), 150.0f);
		ProfilerSubsystem->RecordComponentSample(TEXT("CompA"), 200.0f, 4096);
		ProfilerSubsystem->RecordComponentSample(TEXT("CompB"), 50.0f, 1024);

		FSBPerformanceProfilerMetrics Metrics = ProfilerSubsystem->GetMetrics();
		TestEqual("Total components tracked is 2", Metrics.TotalComponentsTracked, 2);
		TestEqual("Total samples recorded is 2", Metrics.TotalProfilingSamplesRecorded, 2);
		TestEqual("Total frame budget spent is 250.0", Metrics.TotalFrameBudgetSpentUs, 250.0f);
		TestEqual("Hot components count is 1", Metrics.HotComponentsCount, 1);

		TArray<FSBComponentSampleData> HotList = ProfilerSubsystem->GetHotComponents(100.0f);
		TestEqual("Hot components query returned 1 item", HotList.Num(), 1);
		TestEqual("Hot component is CompA", HotList[0].ComponentName, FName(TEXT("CompA")));
	});

	It("Should auto-subscribe performance instrument component, record execution, trigger budget exceeded tag, and cleanup", [this]()
	{
		TestNotNull("InstrumentComp valid", InstrumentComp);
		TestNotNull("StateComp valid", StateComp);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		InstrumentComp->TrackedComponentName = TEXT("PhysicsComponent");
		InstrumentComp->BudgetThresholdUs = 50.0f;
		InstrumentComp->OnInitialize_Implementation();

		TestTrue("Instrumented tag granted", StateComp->HasTag(Tags.State_Profiler_Instrumented));
		TestTrue("SamplingActive tag granted", StateComp->HasTag(Tags.State_Profiler_SamplingActive));
		TestFalse("BudgetExceeded tag not present initially", StateComp->HasTag(Tags.State_Profiler_BudgetExceeded));

		// Execução normal (30us)
		InstrumentComp->RecordExecution(30.0f, 1024);
		TestEqual("Local execution count is 1", InstrumentComp->GetLocalExecutionCount(), 1);
		TestFalse("BudgetExceeded tag still not present", StateComp->HasTag(Tags.State_Profiler_BudgetExceeded));

		// Execução pesada (90us > 50us)
		InstrumentComp->RecordExecution(90.0f, 1024);
		TestEqual("Local execution count is 2", InstrumentComp->GetLocalExecutionCount(), 2);
		TestTrue("BudgetExceeded tag granted after overrun", StateComp->HasTag(Tags.State_Profiler_BudgetExceeded));

		// Shutdown
		InstrumentComp->OnShutdown_Implementation();
		TestFalse("Instrumented tag removed on shutdown", StateComp->HasTag(Tags.State_Profiler_Instrumented));
		TestFalse("BudgetExceeded tag removed on shutdown", StateComp->HasTag(Tags.State_Profiler_BudgetExceeded));
		TestFalse("SamplingActive tag removed on shutdown", StateComp->HasTag(Tags.State_Profiler_SamplingActive));
	});
}
