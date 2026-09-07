#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Subsystems/SBLockFreeEventSubsystem.h"
#include "Components/SBLockFreeProducerComponent.h"
#include "SBCoreTestTypes.h"
#include "SBGameplayTags.h"
#include "GameFramework/Actor.h"
#include "Async/ParallelFor.h"

BEGIN_DEFINE_SPEC(FSBLockFreeQueueTestsSpec, "Sandbox.Core.LockFreeQueue", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	UWorld* TestWorld = nullptr;
	USBLockFreeEventSubsystem* Subsystem = nullptr;
	AActor* TestActor = nullptr;
	USBCoreTestStateComponent* StateComp = nullptr;
	USBLockFreeProducerComponent* ProducerComp = nullptr;
END_DEFINE_SPEC(FSBLockFreeQueueTestsSpec)

void FSBLockFreeQueueTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("LockFreeTestWorld"));
		if (TestWorld)
		{
			Subsystem = TestWorld->GetSubsystem<USBLockFreeEventSubsystem>();
			if (Subsystem)
			{
				Subsystem->InitializeQueue(128);
			}

			TestActor = TestWorld->SpawnActor<AActor>();
			if (TestActor)
			{
				StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
				TestActor->AddInstanceComponent(StateComp);
				StateComp->RegisterComponent();

				ProducerComp = NewObject<USBLockFreeProducerComponent>(TestActor, TEXT("ProducerComp"));
				TestActor->AddInstanceComponent(ProducerComp);
				ProducerComp->RegisterComponent();
			}
		}
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
			Subsystem = nullptr;
			TestActor = nullptr;
			StateComp = nullptr;
			ProducerComp = nullptr;
		}
	});

	It("Should enqueue and dequeue events maintaining strict FIFO ordering and wrap-around ring buffer behavior", [this]()
	{
		TestNotNull("Subsystem valid", Subsystem);
		Subsystem->InitializeQueue(8);

		// Enfileira 5 eventos
		for (int32 i = 1; i <= 5; ++i)
		{
			FSBLockFreeEvent Evt(i, 10, 1, (float)i * 10.0f);
			TestTrue(FString::Printf(TEXT("Enqueue %d"), i), Subsystem->EnqueueEvent(Evt));
		}

		TestEqual("Pending count is 5", Subsystem->GetPendingCount(), 5);

		// Desenfileira 3 eventos
		for (int32 i = 1; i <= 3; ++i)
		{
			FSBLockFreeEvent OutEvt;
			TestTrue(FString::Printf(TEXT("Dequeue %d"), i), Subsystem->DequeueEvent(OutEvt));
			TestEqual(FString::Printf(TEXT("EventID FIFO match %d"), i), OutEvt.EventID, i);
		}

		TestEqual("Pending count after 3 dequeues is 2", Subsystem->GetPendingCount(), 2);

		// Enfileira mais 4 eventos para forçar wrap-around circular
		for (int32 i = 6; i <= 9; ++i)
		{
			FSBLockFreeEvent Evt(i, 10, 1, (float)i * 10.0f);
			TestTrue(FString::Printf(TEXT("Enqueue wrap %d"), i), Subsystem->EnqueueEvent(Evt));
		}

		FSBLockFreeQueueMetrics Metrics = Subsystem->GetMetrics();
		TestEqual("Enqueued total is 9", Metrics.EnqueuedEventsCount, 9);
		TestEqual("Dequeued total is 3", Metrics.DequeuedEventsCount, 3);
		TestEqual("Pending count is 6", Metrics.PendingCount, 6);

		// Desenfileira todos os 6 eventos restantes e verifica ordem contínua 4, 5, 6, 7, 8, 9
		for (int32 i = 4; i <= 9; ++i)
		{
			FSBLockFreeEvent OutEvt;
			TestTrue(FString::Printf(TEXT("Dequeue remaining %d"), i), Subsystem->DequeueEvent(OutEvt));
			TestEqual(FString::Printf(TEXT("EventID FIFO match %d"), i), OutEvt.EventID, i);
		}

		TestEqual("Queue is fully empty", Subsystem->GetPendingCount(), 0);
	});

	It("Should handle multithreaded concurrent enqueue from background workers and drain safely in GameThread", [this]()
	{
		TestNotNull("Subsystem valid", Subsystem);
		Subsystem->InitializeQueue(2000);

		// Dispara 1000 inserções em paralelo
		ParallelFor(1000, [this](int32 Index)
		{
			FSBLockFreeEvent Evt(Index + 1, 100, 2, (float)Index);
			Subsystem->EnqueueEvent(Evt);
		});

		TestEqual("Pending count is 1000 after parallel enqueue", Subsystem->GetPendingCount(), 1000);

		// Drena tudo no GameThread
		TArray<FSBLockFreeEvent> Drained;
		int32 DrainedCount = Subsystem->DrainEvents(Drained, 1500);

		TestEqual("Drained count is 1000", DrainedCount, 1000);
		TestEqual("Drained array size is 1000", Drained.Num(), 1000);
		TestEqual("Pending count after drain is 0", Subsystem->GetPendingCount(), 0);
	});

	It("Should detect buffer saturation, record dropped count correctly, and prevent queue overflow", [this]()
	{
		TestNotNull("Subsystem valid", Subsystem);
		// 16 e a capacidade minima da fila em anel; pedir menos era silenciosamente elevado
		// para 16 e o teste de saturacao nunca saturava.
		Subsystem->InitializeQueue(16);

		// Enche a fila até a capacidade máxima de 16
		for (int32 i = 1; i <= 16; ++i)
		{
			FSBLockFreeEvent Evt(i, 20, 1, 100.0f);
			TestTrue(FString::Printf(TEXT("Enqueue fill %d"), i), Subsystem->EnqueueEvent(Evt));
		}

		TestEqual("Pending count is 16", Subsystem->GetPendingCount(), 16);

		// Tenta inserir mais 5 além do limite (devem ser descartados)
		for (int32 i = 17; i <= 21; ++i)
		{
			FSBLockFreeEvent Evt(i, 20, 1, 100.0f);
			TestFalse(FString::Printf(TEXT("Enqueue overflow rejected %d"), i), Subsystem->EnqueueEvent(Evt));
		}

		FSBLockFreeQueueMetrics Metrics = Subsystem->GetMetrics();
		TestEqual("Dropped events count is 5", Metrics.DroppedEventsCount, 5);
		TestTrue("bIsOverflown is true", Metrics.bIsOverflown);
		TestEqual("Pending count remains 16", Metrics.PendingCount, 16);

		// Desenfileira os 16 eventos originais sem corrupção
		TArray<FSBLockFreeEvent> Drained;
		int32 DrainedCount = Subsystem->DrainEvents(Drained, 32);
		TestEqual("Drained all 16 valid events", DrainedCount, 16);
		TestEqual("Queue empty after drain", Subsystem->GetPendingCount(), 0);
	});

	It("Should auto-register actor with LockFreeProducerComponent, emit events, grant state tags, and trigger delegates", [this]()
	{
		TestNotNull("ProducerComp valid", ProducerComp);
		TestNotNull("StateComp valid", StateComp);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ProducerComp->OnInitialize_Implementation();
		TestTrue("State.LockFree.Active tag granted", StateComp->HasTag(Tags.State_LockFree_Active));

		USBCoreTestLockFreeListener* Listener = NewObject<USBCoreTestLockFreeListener>(ProducerComp);
		ProducerComp->OnEventProduced.AddDynamic(Listener, &USBCoreTestLockFreeListener::HandleEventProduced);

		bool bProduced = ProducerComp->ProduceEvent(55, 99.5f);
		TestTrue("ProduceEvent returned true", bProduced);
		TestTrue("Delegate fired", Listener->bFired);
		TestEqual("Broadcast EventID is 1", Listener->CapturedEventID, 1);
		TestEqual("Broadcast Type is 55", Listener->CapturedEventType, 55);
		TestNearlyEqual("Broadcast Value matches", Listener->CapturedValue, 99.5f, 0.01f);
		TestEqual("Producer count is 1", ProducerComp->GetProducedCount(), 1);
		TestTrue("State.LockFree.Buffering tag granted", StateComp->HasTag(Tags.State_LockFree_Buffering));
	});
}
