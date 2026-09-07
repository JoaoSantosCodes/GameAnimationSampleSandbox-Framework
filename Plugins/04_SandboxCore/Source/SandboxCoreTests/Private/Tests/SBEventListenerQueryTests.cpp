// Copyright 2026 João Santos. All Rights Reserved.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Subsystems/SBEventSubsystem.h"
#include "SBGameplayTags.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSBEventListenerQueryTest, "Sandbox.Events.ListenerQuery", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSBEventListenerQueryTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	GameInstance->InitializeStandalone();
	World->InitializeActorsForPlay(FURL());

	USBEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<USBEventSubsystem>();
	UTEST_NOT_NULL(TEXT("EventSubsystem should be valid"), EventSubsystem);

	const FGameplayTag EventTag = FSBGameplayTags::Get().Event_Attribute_Changed;
	const FGameplayTag OtherTag = FSBGameplayTags::Get().Event_Feature_Toggled;

	UTEST_FALSE(TEXT("Sem inscricao, HasListeners deve ser falso"), EventSubsystem->HasListeners(EventTag));

	int32 ReceivedCount = 0;
	FDelegateHandle Handle = EventSubsystem->SubscribeToEventNative(EventTag, ESBEventPriority::Medium,
		FSBNativeEventDelegate::CreateLambda([&](const FGameplayTag& TriggerTag, UObject* Payload)
		{
			++ReceivedCount;
		}));

	UTEST_TRUE(TEXT("Com inscricao, HasListeners deve ser verdadeiro"), EventSubsystem->HasListeners(EventTag));
	UTEST_FALSE(TEXT("HasListeners deve ser por tag, nao global"), EventSubsystem->HasListeners(OtherTag));

	// O contrato so vale se quem responde verdadeiro de fato recebe a publicacao.
	EventSubsystem->PublishEvent(EventTag, nullptr);
	UTEST_EQUAL(TEXT("Ouvinte inscrito deve receber a publicacao"), ReceivedCount, 1);

	EventSubsystem->UnsubscribeFromEventNative(EventTag, Handle);
	UTEST_FALSE(TEXT("Apos desinscrever, HasListeners deve voltar a ser falso"), EventSubsystem->HasListeners(EventTag));

	// E o inverso: quem responde falso nao pode receber nada.
	EventSubsystem->PublishEvent(EventTag, nullptr);
	UTEST_EQUAL(TEXT("Sem ouvinte, publicar nao deve entregar nada"), ReceivedCount, 1);

	GameInstance->Shutdown();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(true);

	return true;
}
