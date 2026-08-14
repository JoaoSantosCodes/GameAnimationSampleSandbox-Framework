#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Character/SBCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Player.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Components/SBStateComponent.h"
#include "Components/SBInteractionComponent.h"
#include "Interfaces/SBInteractableInterface.h"
#include "Subsystems/SBEventSubsystem.h"
#include "GameplayTagsManager.h"

#include "SBInteractionTestTypes.h"

BEGIN_DEFINE_SPEC(FSBInteractionTestsSpec, "Sandbox.Interaction", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBTestStateComponent* StateComponent;
	USBTestInteractionComponent* InteractionComponent;
	USBTestInteractionListener* EventListener;
	
	APlayerController* ActiveController;
	ULocalPlayer* ActiveLocalPlayer;
	
	APlayerController* ActiveController2;
	ULocalPlayer* ActiveLocalPlayer2;

	FGameplayTag InteractingTag;
END_DEFINE_SPEC(FSBInteractionTestsSpec)

void FSBInteractionTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
		GameInstance->InitializeStandalone();

		FWorldContext* WorldContext = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.OwningGameInstance == GameInstance)
			{
				WorldContext = const_cast<FWorldContext*>(&Context);
				break;
			}
		}
		if (!WorldContext)
		{
			FWorldContext& NewContext = GEngine->CreateNewWorldContext(EWorldType::Game);
			NewContext.OwningGameInstance = GameInstance;
			WorldContext = &NewContext;
		}
		if (WorldContext)
		{
			WorldContext->SetCurrentWorld(TestWorld);
		}

		TestWorld->SetGameInstance(GameInstance);

		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (TestCharacter && TestCharacter->GetCharacterMovement())
		{
			TestCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}

		StateComponent = NewObject<USBTestStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();

		InteractionComponent = NewObject<USBTestInteractionComponent>(TestCharacter);
		InteractionComponent->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(InteractionComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(InteractionComponent);

		// Configura o listener de eventos
		EventListener = NewObject<USBTestInteractionListener>();

		if (USBEventSubsystem* EventSubsystem = TestWorld->GetGameInstance()->GetSubsystem<USBEventSubsystem>())
		{
			EventSubsystem->SubscribeToEventNative(
				FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Available")),
				ESBEventPriority::Medium,
				FSBNativeEventDelegate::CreateUObject(EventListener, &USBTestInteractionListener::OnAvailable)
			);
			EventSubsystem->SubscribeToEventNative(
				FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Cleared")),
				ESBEventPriority::Medium,
				FSBNativeEventDelegate::CreateUObject(EventListener, &USBTestInteractionListener::OnCleared)
			);
			EventSubsystem->SubscribeToEventNative(
				FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Started")),
				ESBEventPriority::Medium,
				FSBNativeEventDelegate::CreateUObject(EventListener, &USBTestInteractionListener::OnStarted)
			);
			EventSubsystem->SubscribeToEventNative(
				FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Progress")),
				ESBEventPriority::Medium,
				FSBNativeEventDelegate::CreateUObject(EventListener, &USBTestInteractionListener::OnProgress)
			);
			EventSubsystem->SubscribeToEventNative(
				FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Completed")),
				ESBEventPriority::Medium,
				FSBNativeEventDelegate::CreateUObject(EventListener, &USBTestInteractionListener::OnCompleted)
			);
		}

		InteractingTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Interacting"));
		ActiveController = nullptr;
		ActiveLocalPlayer = nullptr;
		ActiveController2 = nullptr;
		ActiveLocalPlayer2 = nullptr;
	});

	AfterEach([this]()
	{
		if (ActiveController)
		{
			ActiveController->Destroy();
			ActiveController = nullptr;
		}
		if (ActiveLocalPlayer)
		{
			ActiveLocalPlayer->MarkAsGarbage();
			ActiveLocalPlayer = nullptr;
		}
		if (ActiveController2)
		{
			ActiveController2->Destroy();
			ActiveController2 = nullptr;
		}
		if (ActiveLocalPlayer2)
		{
			ActiveLocalPlayer2->MarkAsGarbage();
			ActiveLocalPlayer2 = nullptr;
		}
		if (TestCharacter)
		{
			TestCharacter->Destroy();
			TestCharacter = nullptr;
		}
		if (TestWorld)
		{
			GEngine->DestroyWorldContext(TestWorld);
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Cenário 1: Detecção e Foco por Trace", [this]()
	{
		// Spawn do objeto interativo a 100 unidades à frente
		FActorSpawnParameters SpawnParams;
		ASBTestInteractableActor* InteractableActor = TestWorld->SpawnActor<ASBTestInteractableActor>(
			ASBTestInteractableActor::StaticClass(),
			FVector(100.f, 0.f, 0.f),
			FRotator::ZeroRotator,
			SpawnParams
		);

		APlayerController* LocalPC = TestWorld->SpawnActor<APlayerController>();
		ActiveController = LocalPC;
		ActiveLocalPlayer = NewObject<ULocalPlayer>(GEngine);
		LocalPC->Player = ActiveLocalPlayer;
		
		APlayerState* LocalPlayerState = TestWorld->SpawnActor<APlayerState>();
		LocalPC->PlayerState = LocalPlayerState;
		
		LocalPC->SetPawn(TestCharacter);
		TestCharacter->Test_Possess(LocalPC);
		TestCharacter->SetRole(ROLE_AutonomousProxy);

		// Rotaciona o personagem olhando para frente (direção do objeto)
		TestCharacter->SetActorLocation(FVector::ZeroVector);
		TestCharacter->SetActorRotation(FRotator::ZeroRotator);
		LocalPC->SetControlRotation(FRotator::ZeroRotator);

		// Configura o mock do traço focando no ator
		InteractionComponent->MockInteractable = InteractableActor;

		// Executa escaneamento e valida detecção
		InteractionComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		TestEqual("Deve detectar o ator sob a mira", InteractionComponent->GetCurrentInteractableActor(), static_cast<AActor*>(InteractableActor));
		TestEqual("Deve emitir Event.Interaction.Available", EventListener->AvailableCount, 1);

		// Configura o mock do traço simulando olhar para longe (sem objeto)
		InteractionComponent->MockInteractable = nullptr;

		// Move o foco rotacionando o controle para longe (olhando para trás)
		LocalPC->SetControlRotation(FRotator(0.f, 180.f, 0.f));
		InteractionComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		TestNull("Deve limpar o foco ao olhar para longe", InteractionComponent->GetCurrentInteractableActor());
		TestEqual("Deve emitir Event.Interaction.Cleared", EventListener->ClearedCount, 1);
	});

	It("Cenário 2: Interação Instantânea (Discreta)", [this]()
	{
		FActorSpawnParameters SpawnParams;
		ASBTestInteractableActor* InteractableActor = TestWorld->SpawnActor<ASBTestInteractableActor>(
			ASBTestInteractableActor::StaticClass(),
			FVector(100.f, 0.f, 0.f),
			FRotator::ZeroRotator,
			SpawnParams
		);
		InteractableActor->CustomDuration = 0.0f; // Instantâneo

		// O personagem do teste por padrão tem autoridade (ROLE_Authority)
		TestCharacter->SetActorLocation(FVector::ZeroVector);
		TestCharacter->SetRole(ROLE_Authority);

		// Configura o mock do traço focando no ator
		InteractionComponent->MockInteractable = InteractableActor;
		InteractionComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		// Dispara interação
		InteractionComponent->StartInteract();

		TestEqual("Deve concluir imediatamente incrementando contador no Servidor", InteractableActor->InteractCount, 1);
		TestFalse("Tag Interacting não deve permanecer activa", StateComponent->HasTag(InteractingTag));
		TestEqual("Deve emitir Event.Interaction.Completed", EventListener->CompletedCount, 1);
	});

	It("Cenário 3: Interação por Retenção (Hold-to-Interact)", [this]()
	{
		FActorSpawnParameters SpawnParams;
		ASBTestInteractableActor* InteractableActor = TestWorld->SpawnActor<ASBTestInteractableActor>(
			ASBTestInteractableActor::StaticClass(),
			FVector(100.f, 0.f, 0.f),
			FRotator::ZeroRotator,
			SpawnParams
		);
		InteractableActor->CustomDuration = 1.0f; // Hold de 1 segundo

		// O personagem do teste por padrão tem autoridade (ROLE_Authority)
		TestCharacter->SetActorLocation(FVector::ZeroVector);
		TestCharacter->SetRole(ROLE_Authority);

		// Configura o mock do traço focando no ator
		InteractionComponent->MockInteractable = InteractableActor;
		InteractionComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		// Inicia Hold
		InteractionComponent->StartInteract();

		TestTrue("Deve entrar em estado de segurando hold", InteractionComponent->IsHoldingInteraction());
		TestTrue("Tag Interacting deve estar ativa no personagem", StateComponent->HasTag(InteractingTag));
		TestEqual("Deve emitir Event.Interaction.Started", EventListener->StartedCount, 1);

		// Tick de 0.5 segundos (50%)
		TestWorld->Tick(LEVELTICK_All, 0.25f);
		InteractionComponent->TickComponent(0.25f, LEVELTICK_All, nullptr);
		TestWorld->Tick(LEVELTICK_All, 0.25f);
		InteractionComponent->TickComponent(0.25f, LEVELTICK_All, nullptr);
		TestEqual("Progresso deve reportar 0.5", InteractionComponent->GetHoldProgressPercent(), 0.5f);
		TestEqual("Deve despachar Event.Interaction.Progress", EventListener->ProgressCount, 2);
		TestEqual("Valor no payload de progresso deve ser 0.5", EventListener->LastProgressVal, 0.5f);
		TestEqual("Objeto interativo não deve ter sido acionado ainda", InteractableActor->InteractCount, 0);

		// Tick de mais 0.5 segundos (conclusão)
		TestWorld->Tick(LEVELTICK_All, 0.25f);
		InteractionComponent->TickComponent(0.25f, LEVELTICK_All, nullptr);
		TestWorld->Tick(LEVELTICK_All, 0.25f);
		InteractionComponent->TickComponent(0.25f, LEVELTICK_All, nullptr);
		TestEqual("Objeto interativo deve ser executado no final do hold", InteractableActor->InteractCount, 1);
		TestFalse("Tag Interacting deve ser removida", StateComponent->HasTag(InteractingTag));
		TestFalse("Hold deve ser desmarcado como ativo", InteractionComponent->IsHoldingInteraction());
		TestEqual("Deve emitir Event.Interaction.Completed", EventListener->CompletedCount, 1);
	});

	It("Cenário 4: Interrupção por Distância (Network Safety)", [this]()
	{
		FActorSpawnParameters SpawnParams;
		ASBTestInteractableActor* InteractableActor = TestWorld->SpawnActor<ASBTestInteractableActor>(
			ASBTestInteractableActor::StaticClass(),
			FVector(100.f, 0.f, 0.f),
			FRotator::ZeroRotator,
			SpawnParams
		);
		InteractableActor->CustomDuration = 1.0f;

		// O personagem do teste por padrão tem autoridade (ROLE_Authority)
		TestCharacter->SetActorLocation(FVector::ZeroVector);
		TestCharacter->SetRole(ROLE_Authority);

		// Configura o mock do traço focando no ator
		InteractionComponent->MockInteractable = InteractableActor;
		InteractionComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		InteractionComponent->StartInteract();
		TestTrue("Hold ativo", InteractionComponent->IsHoldingInteraction());

		// Teleporta o jogador para longe (fora do alcance de 250 unidades)
		TestCharacter->SetActorLocation(FVector(500.f, 0.f, 0.f));

		// Tick verifica a distância e interrompe
		InteractionComponent->TickComponent(0.1f, LEVELTICK_All, nullptr);

		TestFalse("Hold deve ter sido interrompido por distância", InteractionComponent->IsHoldingInteraction());
		TestFalse("Tag Interacting deve ter sido limpa", StateComponent->HasTag(InteractingTag));
		TestEqual("Objeto não deve ser acionado", InteractableActor->InteractCount, 0);
	});

	It("Cenário 5: Condição de Corrida de Alvo Compartilhado (Race Condition)", [this]()
	{
		// Spawn do objeto compartilhado
		FActorSpawnParameters SpawnParams;
		ASBTestInteractableActor* SharedInteractable = TestWorld->SpawnActor<ASBTestInteractableActor>(
			ASBTestInteractableActor::StaticClass(),
			FVector(100.f, 0.f, 0.f),
			FRotator::ZeroRotator,
			SpawnParams
		);
		SharedInteractable->CustomDuration = 2.0f; // Tempo de hold

		// Personagem 1 (Jogador 1 - Servidor/Autoridade)
		TestCharacter->SetActorLocation(FVector::ZeroVector);
		TestCharacter->SetRole(ROLE_Authority);

		// Personagem 2 (Jogador 2 - Cliente Simulador)
		ASBCharacter* TestCharacter2 = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector(0.f, 50.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		TestCharacter2->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		TestCharacter2->SetRole(ROLE_AutonomousProxy);
		
		USBTestStateComponent* StateComponent2 = NewObject<USBTestStateComponent>(TestCharacter2);
		StateComponent2->RegisterComponent();
		USBTestInteractionComponent* InteractionComponent2 = NewObject<USBTestInteractionComponent>(TestCharacter2);
		InteractionComponent2->RegisterComponent();
		
		ISBComponentInterface::Execute_OnInitialize(StateComponent2);
		ISBComponentInterface::Execute_OnInitialize(InteractionComponent2);
		ISBComponentInterface::Execute_OnReady(StateComponent2);
		ISBComponentInterface::Execute_OnReady(InteractionComponent2);

		// --- EXECUÇÃO SIMULTÂNEA NO SERVIDOR ---
		
		// Ambos os componentes focam no mesmo objeto compartilhado via mock
		InteractionComponent->MockInteractable = SharedInteractable;
		InteractionComponent2->MockInteractable = SharedInteractable;

		// 1. Ambos os componentes escaneiam e focam no objeto
		InteractionComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);
		InteractionComponent2->TickComponent(0.016f, LEVELTICK_All, nullptr);
		
		// 2. Jogador 1 inicia interação (autoritativo, executa imediatamente)
		InteractionComponent->StartInteract();
		
		// 3. Jogador 2 (cliente) inicia hold localmente
		InteractionComponent2->StartInteract();
		
		// 4. Servidor recebe e processa a requisição do Jogador 2 (chamamos a implementação do RPC de forma simulada)
		InteractionComponent2->ServerStartInteract_Implementation(SharedInteractable);
		
		// 5. Como o lock falhou, o servidor envia o cancelamento. Chamamos o handler de cancelamento de cliente diretamente
		InteractionComponent2->ClientCancelInteraction_Implementation();

		// VERIFICAÇÕES DE LOCK:
		TestTrue("LockOwner no objeto deve ser o Jogador 1 (TestCharacter)", SharedInteractable->CurrentLockOwner == TestCharacter);
		TestEqual("Interação ativa no Jogador 1", InteractionComponent->GetCurrentInteractableActor(), static_cast<AActor*>(SharedInteractable));
		
		// O Jogador 2 deve ser cancelado/rejeitado ejetando seu hold
		TestFalse("Jogador 2 deve ser rejeitado pelo Lock", InteractionComponent2->IsHoldingInteraction());
		TestFalse("Tag Interacting do Jogador 2 deve ser limpa", StateComponent2->HasTag(InteractingTag));
 
		// Limpa o Personagem 2
		TestCharacter2->Destroy();
	});

	It("Cenário 6: Validação de Segurança de Rede (Anti-Cheat & Rate-Limiting)", [this]()
	{
		// 1. Rejeita Target nulo
		TestFalse("ServerStartInteract_Validate deve rejeitar Target nulo", InteractionComponent->ServerStartInteract_Validate(nullptr));

		// 2. Rejeita objeto muito distante (fora do alcance de 250 + 150 de tolerância)
		FActorSpawnParameters SpawnParams;
		ASBTestInteractableActor* DistantInteractable = TestWorld->SpawnActor<ASBTestInteractableActor>(
			ASBTestInteractableActor::StaticClass(),
			FVector(1000.f, 0.f, 0.f), // 1000 unidades de distância (limite é 400)
			FRotator::ZeroRotator,
			SpawnParams
		);

		TestFalse("ServerStartInteract_Validate deve rejeitar Target fora do alcance", InteractionComponent->ServerStartInteract_Validate(DistantInteractable));

		// 3. Permite objeto próximo (deve validar com sucesso no primeiro frame)
		ASBTestInteractableActor* NearInteractable = TestWorld->SpawnActor<ASBTestInteractableActor>(
			ASBTestInteractableActor::StaticClass(),
			FVector(100.f, 0.f, 0.f), // 100 unidades de distância (dentro do alcance)
			FRotator::ZeroRotator,
			SpawnParams
		);

		TestTrue("ServerStartInteract_Validate deve permitir Target dentro do alcance", InteractionComponent->ServerStartInteract_Validate(NearInteractable));

		// 4. Rate Limiter (limite de 10 chamadas por segundo)
		// A chamada "Near" anterior foi bem sucedida (contando como 1).
		// Duas chamadas anteriores falharam na validação lógica mas contaram como tentativas no rate limiter (Total = 3).
		// Restam 7 tentativas bem-sucedidas permitidas na janela de 10.
		int32 SuccessfulCalls = 1;
		for (int32 i = 0; i < 15; ++i)
		{
			if (InteractionComponent->ServerStartInteract_Validate(NearInteractable))
			{
				SuccessfulCalls++;
			}
		}

		TestEqual("Deve permitir no máximo 8 chamadas bem-sucedidas devido às tentativas consumidas (total 10)", SuccessfulCalls, 8);
		
		// Limpeza
		DistantInteractable->Destroy();
		NearInteractable->Destroy();
	});
}
