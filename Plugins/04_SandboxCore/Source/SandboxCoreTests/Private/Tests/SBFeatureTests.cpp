// Copyright 2026 João Santos. All Rights Reserved.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Subsystems/SBSandboxFeatureSubsystem.h"
#include "Subsystems/SBEventSubsystem.h"
#include "SBGameplayTags.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSBFeatureSubsystemTest, "Sandbox.Features.SubsystemVerification", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSBFeatureSubsystemTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	GameInstance->InitializeStandalone();
	World->InitializeActorsForPlay(FURL());

	USBSandboxFeatureSubsystem* FeatureSubsystem = GameInstance->GetSubsystem<USBSandboxFeatureSubsystem>();
	UTEST_NOT_NULL(TEXT("FeatureSubsystem should be valid"), FeatureSubsystem);

	USBEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<USBEventSubsystem>();
	UTEST_NOT_NULL(TEXT("EventSubsystem should be valid"), EventSubsystem);

	FGameplayTag FeatureTag = FSBGameplayTags::Get().Feature_Combat;
	UTEST_FALSE(TEXT("Feature should not be active initially"), FeatureSubsystem->IsFeatureEnabled(FeatureTag));

	bool bEventFired = false;
	FGameplayTag ReceivedTag;
	bool bReceivedEnabled = false;

	FGameplayTag EventTag = FSBGameplayTags::Get().Event_Feature_Toggled;
	EventSubsystem->SubscribeToEventNative(EventTag, ESBEventPriority::Medium, FSBNativeEventDelegate::CreateLambda([&](const FGameplayTag& TriggerTag, UObject* Payload)
	{
		USBSandboxFeaturePayload* FeaturePayload = Cast<USBSandboxFeaturePayload>(Payload);
		if (FeaturePayload)
		{
			bEventFired = true;
			ReceivedTag = FeaturePayload->FeatureTag;
			bReceivedEnabled = FeaturePayload->bEnabled;
		}
	}));

	FeatureSubsystem->EnableFeature(FeatureTag);
	UTEST_TRUE(TEXT("Feature should be active after enabling"), FeatureSubsystem->IsFeatureEnabled(FeatureTag));
	UTEST_TRUE(TEXT("Event should have fired on event bus"), bEventFired);
	UTEST_TRUE(TEXT("Payload should indicate feature enabled"), bReceivedEnabled);
	UTEST_EQUAL(TEXT("Payload feature tag should match"), ReceivedTag, FeatureTag);

	bEventFired = false;
	FeatureSubsystem->DisableFeature(FeatureTag);
	UTEST_FALSE(TEXT("Feature should not be active after disabling"), FeatureSubsystem->IsFeatureEnabled(FeatureTag));
	UTEST_TRUE(TEXT("Event should have fired on disabling"), bEventFired);
	UTEST_FALSE(TEXT("Payload should indicate feature disabled"), bReceivedEnabled);

	GameInstance->Shutdown();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(true);

	return true;
}
