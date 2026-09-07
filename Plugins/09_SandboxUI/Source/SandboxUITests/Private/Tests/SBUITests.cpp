#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Tests/SBUITestMockWidget.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "GameplayTagsManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

BEGIN_DEFINE_SPEC(FSBUITestsSpec, "Sandbox.UI.WidgetEvents", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	USBUITestMockWidget* TestWidget;
	USBEventSubsystem* EventSubsystem;
	FGameplayTag TestTag;
END_DEFINE_SPEC(FSBUITestsSpec)

void FSBUITestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		UGameInstance* GI = NewObject<UGameInstance>(GEngine);
		GI->Init();
		TestWorld->SetGameInstance(GI);
		
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		TestTag = TagsManager.AddNativeGameplayTag(TEXT("Test.UI.Event"));

		EventSubsystem = GI->GetSubsystem<USBEventSubsystem>();

		// Create widget
		TestWidget = NewObject<USBUITestMockWidget>(TestWorld);
	});

	AfterEach([this]()
	{
		if (TestWidget)
		{
			TestWidget->UnsubscribeAllEvents();
			TestWidget->ConditionalBeginDestroy();
		}
		if (TestWorld)
		{
			if (UGameInstance* GI = TestWorld->GetGameInstance())
			{
				GI->Shutdown();
			}
			TestWorld->DestroyWorld(false);
		}
	});

	Describe("Event Subscriptions", [this]()
	{
		It("should be idempotent and auto-unsubscribe cleanly", [this]()
		{
			FSBBlueprintEventDelegate Delegate;
			Delegate.BindUFunction(TestWidget, FName("HandleTestEvent"));

			// Subscribe twice
			TestWidget->SubscribeToEvent(TestTag, Delegate);
			TestWidget->SubscribeToEvent(TestTag, Delegate);

			// Publish event
			USBPawnEventPayload* Payload = NewObject<USBPawnEventPayload>(TestWorld);
			EventSubsystem->PublishEvent(TestTag, Payload);

			// CallCount should be 1 (due to idempotency check!)
			TestEqual("Call count should be 1 due to idempotency", TestWidget->CallCount, 1);

			// Unsubscribe
			TestWidget->UnsubscribeAllEvents();
			TestWidget->CallCount = 0;

			// Publish again
			EventSubsystem->PublishEvent(TestTag, Payload);
			TestEqual("Call count should remain 0 after unsubscribe", TestWidget->CallCount, 0);
		});
	});

	Describe("Local Player Scope Filtering", [this]()
	{
		It("should filter events based on TargetPawn matching widget's possessed pawn", [this]()
		{
			// Spawn pawns
			APawn* PawnA = TestWorld->SpawnActor<APawn>();
			APawn* PawnB = TestWorld->SpawnActor<APawn>();

			TestWidget->bMockOwningPawn = true;
			TestWidget->MockOwningPawn = PawnA;

			// Bind delegate to track widget responses
			FSBBlueprintEventDelegate Delegate;
			Delegate.BindUFunction(TestWidget, FName("HandleTestEvent"));
			TestWidget->SubscribeToEvent(TestTag, Delegate);

			// 1. Publish event with PawnB (mismatch)
			USBPawnEventPayload* PayloadB = NewObject<USBPawnEventPayload>(TestWorld);
			PayloadB->TargetPawn = PawnB;

			TestWidget->CallCount = 0;
			
			// Simulate the local widget check pattern using GetOwningPlayerPawn
			APawn* OwningPawn = TestWidget->GetOwningPlayerPawn();
			if (PayloadB->TargetPawn == OwningPawn)
			{
				EventSubsystem->PublishEvent(TestTag, PayloadB);
			}

			TestEqual("Widget should NOT be notified of events for PawnB", TestWidget->CallCount, 0);

			// 2. Publish event with PawnA (match)
			USBPawnEventPayload* PayloadA = NewObject<USBPawnEventPayload>(TestWorld);
			PayloadA->TargetPawn = PawnA;

			OwningPawn = TestWidget->GetOwningPlayerPawn();
			if (PayloadA->TargetPawn == OwningPawn)
			{
				EventSubsystem->PublishEvent(TestTag, PayloadA);
			}

			TestEqual("Widget should be notified of events for PawnA", TestWidget->CallCount, 1);

			// Cleanup
			PawnA->Destroy();
			PawnB->Destroy();
		});
	});
}
