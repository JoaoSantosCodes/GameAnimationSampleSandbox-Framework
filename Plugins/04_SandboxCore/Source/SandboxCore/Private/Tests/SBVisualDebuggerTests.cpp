#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Subsystems/SBVisualDebuggerSubsystem.h"
#include "Components/SBVisualDebugOverlayComponent.h"
#include "SBCoreTestTypes.h"
#include "SBGameplayTags.h"
#include "GameFramework/Actor.h"

BEGIN_DEFINE_SPEC(FSBVisualDebuggerTestsSpec, "Sandbox.Core.VisualDebugger", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	UWorld* TestWorld = nullptr;
	USBVisualDebuggerSubsystem* DebugSubsystem = nullptr;
	AActor* TestActor = nullptr;
	USBCoreTestStateComponent* StateComp = nullptr;
	USBVisualDebugOverlayComponent* OverlayComp = nullptr;
END_DEFINE_SPEC(FSBVisualDebuggerTestsSpec)

void FSBVisualDebuggerTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("VisualDebuggerTestWorld"));
		if (TestWorld)
		{
			DebugSubsystem = TestWorld->GetSubsystem<USBVisualDebuggerSubsystem>();
			if (DebugSubsystem)
			{
				DebugSubsystem->ResetSubsystem();
			}

			TestActor = TestWorld->SpawnActor<AActor>();
			if (TestActor)
			{
				TestActor->SetActorLocation(FVector(0.0f, 0.0f, 100.0f));

				StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
				TestActor->AddInstanceComponent(StateComp);
				StateComp->RegisterComponent();

				OverlayComp = NewObject<USBVisualDebugOverlayComponent>(TestActor, TEXT("OverlayComp"));
				TestActor->AddInstanceComponent(OverlayComp);
				OverlayComp->RegisterComponent();
			}
		}
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
			DebugSubsystem = nullptr;
			TestActor = nullptr;
			StateComp = nullptr;
			OverlayComp = nullptr;
		}
	});

	It("Should toggle category overlay and query enabled state with broadcast", [this]()
	{
		TestNotNull("DebugSubsystem valid", DebugSubsystem);

		TestFalse("PowerGrid initially disabled", DebugSubsystem->IsCategoryEnabled(ESBOverlayCategory::PowerGrid));
		TestFalse("PipeNetwork initially disabled", DebugSubsystem->IsCategoryEnabled(ESBOverlayCategory::PipeNetwork));

		DebugSubsystem->SetCategoryEnabled(ESBOverlayCategory::PowerGrid, true);
		TestTrue("PowerGrid now enabled", DebugSubsystem->IsCategoryEnabled(ESBOverlayCategory::PowerGrid));
		TestFalse("PipeNetwork still disabled", DebugSubsystem->IsCategoryEnabled(ESBOverlayCategory::PipeNetwork));

		DebugSubsystem->SetCategoryEnabled(ESBOverlayCategory::All, true);
		TestTrue("All enabled - PowerGrid active", DebugSubsystem->IsCategoryEnabled(ESBOverlayCategory::PowerGrid));
		TestTrue("All enabled - PipeNetwork active", DebugSubsystem->IsCategoryEnabled(ESBOverlayCategory::PipeNetwork));
		TestTrue("All enabled - DroneRoutes active", DebugSubsystem->IsCategoryEnabled(ESBOverlayCategory::DroneRoutes));
	});

	It("Should queue render items and filter by enabled category", [this]()
	{
		TestNotNull("DebugSubsystem valid", DebugSubsystem);

		DebugSubsystem->SetCategoryEnabled(ESBOverlayCategory::PowerGrid, true);
		DebugSubsystem->SetCategoryEnabled(ESBOverlayCategory::DroneRoutes, false);

		FSBOverlayRenderItem PowerLine(FVector::ZeroVector, FVector(100, 0, 0), FColor::Cyan, TEXT("Power_1"), ESBOverlayCategory::PowerGrid);
		FSBOverlayRenderItem DronePath(FVector::ZeroVector, FVector(0, 500, 200), FColor::Orange, TEXT("Drone_Route_A"), ESBOverlayCategory::DroneRoutes);

		DebugSubsystem->QueueRenderItem(PowerLine);
		DebugSubsystem->QueueRenderItem(DronePath);

		TArray<FSBOverlayRenderItem> PowerItems = DebugSubsystem->GetQueuedRenderItems(ESBOverlayCategory::PowerGrid);
		TestEqual("PowerGrid items queued is 1", PowerItems.Num(), 1);
		TestEqual("Power item label matches", PowerItems[0].DebugText, FString(TEXT("Power_1")));

		TArray<FSBOverlayRenderItem> DroneItems = DebugSubsystem->GetQueuedRenderItems(ESBOverlayCategory::DroneRoutes);
		TestEqual("DroneRoutes items queued is 0 (was disabled)", DroneItems.Num(), 0);

		DebugSubsystem->ClearRenderItems();
		TestEqual("Cleared render items count is 0", DebugSubsystem->GetQueuedRenderItems(ESBOverlayCategory::All).Num(), 0);
	});

	It("Should track telemetry metrics for render items and enabled categories mask", [this]()
	{
		TestNotNull("DebugSubsystem valid", DebugSubsystem);

		DebugSubsystem->SetCategoryEnabled(ESBOverlayCategory::PowerGrid, true);
		DebugSubsystem->SetCategoryEnabled(ESBOverlayCategory::PipeNetwork, true);

		FSBVisualDebuggerMetrics Metrics = DebugSubsystem->GetMetrics();
		TestEqual("Active overlays count is 2", Metrics.ActiveOverlaysCount, 2);

		FSBOverlayRenderItem ItemA(FVector::ZeroVector, FVector(100, 0, 0), FColor::Green, TEXT("A"), ESBOverlayCategory::PowerGrid);
		FSBOverlayRenderItem ItemB(FVector::ZeroVector, FVector(200, 0, 0), FColor::Blue, TEXT("B"), ESBOverlayCategory::PipeNetwork);
		DebugSubsystem->QueueRenderItem(ItemA);
		DebugSubsystem->QueueRenderItem(ItemB);

		FSBVisualDebuggerMetrics MetricsAfter = DebugSubsystem->GetMetrics();
		TestEqual("Total render items queued is 2", MetricsAfter.TotalRenderItemsQueued, 2);
	});

	It("Should auto-subscribe visual debug overlay component, push lines when active, update state tags, and cleanup", [this]()
	{
		TestNotNull("OverlayComp valid", OverlayComp);
		TestNotNull("StateComp valid", StateComp);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		OverlayComp->ComponentCategory = ESBOverlayCategory::PowerGrid;
		OverlayComp->OnInitialize_Implementation();

		TestFalse("Initially not active tag", StateComp->HasTag(Tags.State_Debug_OverlayActive));

		// Ativa no subsistema
		DebugSubsystem->SetCategoryEnabled(ESBOverlayCategory::PowerGrid, true);
		TestTrue("OverlayActive tag granted after toggle", StateComp->HasTag(Tags.State_Debug_OverlayActive));
		TestTrue("VisualizingPower tag granted", StateComp->HasTag(Tags.State_Debug_VisualizingPower));

		// Enfileira linha de debug
		OverlayComp->PushDebugLine(FVector(500, 0, 100), FColor::Yellow, TEXT("MainFeedLine"));
		TestEqual("Pushed lines count is 1", OverlayComp->GetPushedLinesCount(), 1);

		TArray<FSBOverlayRenderItem> Items = DebugSubsystem->GetQueuedRenderItems(ESBOverlayCategory::PowerGrid);
		TestEqual("Subsystem received pushed item", Items.Num(), 1);
		TestEqual("Pushed item label is MainFeedLine", Items[0].DebugText, FString(TEXT("MainFeedLine")));

		// Shutdown
		OverlayComp->OnShutdown_Implementation();
		TestFalse("OverlayActive removed on shutdown", StateComp->HasTag(Tags.State_Debug_OverlayActive));
		TestFalse("VisualizingPower removed on shutdown", StateComp->HasTag(Tags.State_Debug_VisualizingPower));
	});
}
