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
#include "AIController.h"
#include "Components/SBMovementComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Movement/Behaviors/SBMovementBehavior.h"
#include "Movement/Behaviors/SBMovementBehaviorCrouch.h"
#include "Movement/Behaviors/SBMovementBehaviorSprint.h"
#include "Movement/DataAssets/SBMovementConfigDataAsset.h"
#include "Movement/DataAssets/SBMovementBehaviorDefinition.h"
#include "Movement/Aggregator/SBMovementModifierAggregator.h"
#include "GameplayTagsManager.h"

BEGIN_DEFINE_SPEC(FSBNetworkTestsSpec, "Sandbox.Character.Network", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBTestStateComponent* StateComponent;
	USBTestMovementComponent* MovementComponent;
	USBAttributeComponent* AttributeComponent;
	USBMovementConfigDataAsset* ConfigAsset;
	AController* ActiveController;
	ULocalPlayer* ActiveLocalPlayer;

	FGameplayTag CrouchTag;
	FGameplayTag SprintTag;
	FGameplayTag AmmoTag;
END_DEFINE_SPEC(FSBNetworkTestsSpec)

void FSBNetworkTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (TestCharacter && TestCharacter->GetCharacterMovement())
		{
			TestCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
		
		// Instancia componentes de teste
		StateComponent = NewObject<USBTestStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();

		MovementComponent = NewObject<USBTestMovementComponent>(TestCharacter);
		MovementComponent->RegisterComponent();

		AttributeComponent = NewObject<USBAttributeComponent>(TestCharacter);
		AttributeComponent->RegisterComponent();

		// Inicialização
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(MovementComponent);
		ISBComponentInterface::Execute_OnInitialize(AttributeComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(MovementComponent);
		ISBComponentInterface::Execute_OnReady(AttributeComponent);

		// Registra Atributo de Munição de teste
		AmmoTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Weapon.Ammo"));
		FSBAttribute InitialAmmo;
		InitialAmmo.BaseValue = 30.0f;
		InitialAmmo.CurrentValue = 30.0f;
		InitialAmmo.MaxValue = 30.0f;
		InitialAmmo.MinValue = 0.0f;
		AttributeComponent->RegisterAttribute(AmmoTag, InitialAmmo);

		// Registra Atributo de Stamina de teste
		FGameplayTag StaminaTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Stamina"));
		FSBAttribute InitialStamina;
		InitialStamina.BaseValue = 100.0f;
		InitialStamina.CurrentValue = 100.0f;
		InitialStamina.MaxValue = 100.0f;
		InitialStamina.MinValue = 0.0f;
		AttributeComponent->RegisterAttribute(StaminaTag, InitialStamina);

		// Cria tags
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		CrouchTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Crouching"));
		SprintTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Sprinting"));
		TagsManager.AddNativeGameplayTag(TEXT("Movement.Group.Stance"));
		TagsManager.AddNativeGameplayTag(TEXT("Stat.MovementSpeed"));

		// Configura os behaviors e data assets para a movimentação
		ConfigAsset = NewObject<USBMovementConfigDataAsset>();

		USBMovementBehaviorCrouchDefinition* CrouchDef = NewObject<USBMovementBehaviorCrouchDefinition>();
		CrouchDef->BehaviorTag = CrouchTag;
		CrouchDef->StackPriority = 20;
		CrouchDef->ExclusivityGroup = FGameplayTag::RequestGameplayTag(TEXT("Movement.Group.Stance"));
		
		FSBModifierEntry CrouchMod;
		CrouchMod.TargetStatTag = FGameplayTag::RequestGameplayTag(TEXT("Stat.MovementSpeed"));
		CrouchMod.Operation = ESBModifierOperation::Override;
		CrouchMod.Value = 300.f;
		CrouchDef->MovementModifiers.Add(CrouchMod);

		USBMovementBehaviorDefinition* SprintDef = NewObject<USBMovementBehaviorDefinition>();
		SprintDef->BehaviorTag = SprintTag;
		SprintDef->StackPriority = 50;
		SprintDef->ExclusivityGroup = FGameplayTag::RequestGameplayTag(TEXT("Movement.Group.Stance"));

		FSBModifierEntry SprintMod;
		SprintMod.TargetStatTag = FGameplayTag::RequestGameplayTag(TEXT("Stat.MovementSpeed"));
		SprintMod.Operation = ESBModifierOperation::Multiply;
		SprintMod.Value = 1.5f;
		SprintDef->MovementModifiers.Add(SprintMod);

		FSBMovementBehaviorConfigEntry CrouchEntry;
		CrouchEntry.BehaviorClass = USBMovementBehaviorCrouch::StaticClass();
		CrouchEntry.DefinitionAsset = CrouchDef;
		ConfigAsset->ConfiguredBehaviors.Add(CrouchEntry);

		FSBMovementBehaviorConfigEntry SprintEntry;
		SprintEntry.BehaviorClass = USBMovementBehaviorSprint::StaticClass();
		SprintEntry.DefinitionAsset = SprintDef;
		ConfigAsset->ConfiguredBehaviors.Add(SprintEntry);

		MovementComponent->LoadMovementConfig(ConfigAsset);
		
		ActiveController = nullptr;
		ActiveLocalPlayer = nullptr;
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
		if (TestCharacter)
		{
			TestCharacter->Destroy();
			TestCharacter = nullptr;
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Cenário 1: Validação de Entrada de Comportamento e Rejeição do Servidor (Rollback)", [this]()
	{
		APlayerController* LocalPC = TestWorld->SpawnActor<APlayerController>();
		ActiveController = LocalPC;
		ActiveLocalPlayer = NewObject<ULocalPlayer>(GEngine);
		LocalPC->Player = ActiveLocalPlayer;
		APlayerState* LocalPlayerState = TestWorld->SpawnActor<APlayerState>();
		LocalPC->PlayerState = LocalPlayerState;
		LocalPC->SetPawn(TestCharacter);
		TestCharacter->Test_Possess(LocalPC);

		// Configura o Pawn como Cliente Local (Autonomous Proxy) após possessão
		TestCharacter->SetRole(ROLE_AutonomousProxy);



		TestTrue("Personagem deve estar sob controle local", TestCharacter->IsLocallyControlled());
		TestFalse("Personagem cliente nao deve ter autoridade", TestCharacter->HasAuthority());

		// 1. Cliente tenta iniciar Crouch preditivamente
		MovementComponent->RequestBehavior(CrouchTag);

		// A tag de estado predita e o modificador de velocidade devem estar ativos no cliente
		TestTrue("Tag Crouch deve estar no PredictedStateTags do cliente", StateComponent->GetPredictedStateTags().HasTagExact(CrouchTag));
		TestFalse("Tag Crouch não deve estar no ActiveStateTags (servidor ainda não confirmou)", StateComponent->GetActiveStateTags().HasTagExact(CrouchTag));
		TestEqual("Velocidade do cliente deve ser reduzida preditivamente", MovementComponent->GetSpeedModifierAggregator()->CalculateFinalValue(600.f), 300.f);

		// 2. Servidor rejeita e envia RPC ClientStopBehavior de volta
		MovementComponent->ClientStopBehavior_Implementation(CrouchTag);
		MovementComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		// Verificação de Rollback Simétrico
		TestFalse("Tag Crouch deve ser removida de PredictedStateTags após rollback", StateComponent->GetPredictedStateTags().HasTagExact(CrouchTag));
		TestEqual("Velocidade do cliente deve retornar a 600.f (modificador limpo do Aggregator)", MovementComponent->GetSpeedModifierAggregator()->CalculateFinalValue(600.f), 600.f);
	});

	It("Cenário 2: Ejeção por Grupo Autoritativa do Servidor", [this]()
	{
		// Configura o Pawn como Servidor Autoritativo com conexão remota fictícia (IsLocallyControlled == false)
		TestCharacter->SetRole(ROLE_Authority);
		
		APlayerController* RemotePC = TestWorld->SpawnActor<APlayerController>();
		ActiveController = RemotePC;
		APlayerState* RemotePlayerState = TestWorld->SpawnActor<APlayerState>();
		RemotePC->PlayerState = RemotePlayerState;
		RemotePC->SetPawn(TestCharacter);
		TestCharacter->Test_Possess(RemotePC);

		TestFalse("No servidor, o pawn do cliente remoto não é locally controlled", TestCharacter->IsLocallyControlled());
		TestTrue("Servidor tem autoridade", TestCharacter->HasAuthority());

		// 1. Ativa Crouch no servidor
		MovementComponent->RequestBehavior(CrouchTag);
		TestTrue("Crouch ativo no servidor", StateComponent->HasTag(CrouchTag));

		// 2. Servidor ativa Sprint (ejeta o Crouch)
		MovementComponent->RequestBehavior(SprintTag);
		MovementComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		// Crouch deve ser ejetado no servidor
		TestFalse("Crouch deve ser ejetado no servidor", StateComponent->HasTag(CrouchTag));
		TestTrue("Sprint deve assumir a liderança no servidor", StateComponent->HasTag(SprintTag));

		// Verificação de Envio do RPC: O servidor deve ter disparado o ClientStopBehavior para o Crouch ejetado
		TestEqual("O servidor deve propagar a parada do Crouch enviando ClientStopBehavior ao cliente remoto", MovementComponent->LastClientStopBehaviorTag, CrouchTag);
	});

	It("Cenário 3: Supressão de RPCs para NPCs/IAs", [this]()
	{
		// Configura o Pawn como Servidor Autoritativo
		TestCharacter->SetRole(ROLE_Authority);

		// Instancia um Controller de IA (não é jogador real)
		AAIController* AIController = TestWorld->SpawnActor<AAIController>();
		ActiveController = AIController;
		AIController->Possess(TestCharacter);

		TestFalse("AI Pawn não é Locally Controlled", TestCharacter->IsLocallyControlled());
		TestTrue("AI Pawn roda sob autoridade do servidor", TestCharacter->HasAuthority());

		// 1. Ativa Crouch
		MovementComponent->RequestBehavior(CrouchTag);
		TestTrue("Crouch ativo na IA", StateComponent->HasTag(CrouchTag));

		// 2. IA ativa Sprint (ejeta Crouch)
		MovementComponent->RequestBehavior(SprintTag);
		MovementComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		// Verificação: A IA deve processar a ejeção física com sucesso
		TestFalse("Crouch ejetado fisicamente no servidor do NPC", StateComponent->HasTag(CrouchTag));
		TestTrue("Sprint ativo no servidor do NPC", StateComponent->HasTag(SprintTag));
		
		// Verificação de Supressão: Nenhuma RPC ClientStopBehavior deve ter sido enfileirada/enviada para o NPC de IA
		TestTrue("Nenhuma RPC deve ser gerada para IAs (LastClientStopBehaviorTag deve estar vazia)", MovementComponent->LastClientStopBehaviorTag.IsValid() == false);
	});

	It("Cenário 4: Transações de Atributos com Confirmação Jitter-Free (Munição)", [this]()
	{
		APlayerController* LocalPC = TestWorld->SpawnActor<APlayerController>();
		ActiveController = LocalPC;
		ActiveLocalPlayer = NewObject<ULocalPlayer>(GEngine);
		LocalPC->Player = ActiveLocalPlayer;
		APlayerState* LocalPlayerState = TestWorld->SpawnActor<APlayerState>();
		LocalPC->PlayerState = LocalPlayerState;
		LocalPC->SetPawn(TestCharacter);
		TestCharacter->Test_Possess(LocalPC);

		// Configura o Pawn como Cliente Local (Autonomous Proxy) após possessão
		TestCharacter->SetRole(ROLE_AutonomousProxy);

		// 1. Cliente prediz consumo de 1 unidade de munição (ID = 1)
		bool bConsumeSuccess = AttributeComponent->TryConsumeAttribute(AmmoTag, 1.0f, TestCharacter, 1);
		TestTrue("Consumo preditivo de munição deve ser aceito", bConsumeSuccess);

		// HUD deve refletir o valor predito (30 - 1 = 29)
		TestEqual("HUD deve ler 29.0f predito", AttributeComponent->GetAttributeValue(AmmoTag), 29.0f);

		// BaseValue replicado no cliente ainda é 30.f
		FSBAttribute ClientAttr;
		AttributeComponent->GetAttribute(AmmoTag, ClientAttr);
		TestEqual("BaseValue na replicação ainda deve ser 30.f", ClientAttr.BaseValue, 30.0f);

		// 2. Simula o Servidor processando o consumo e atualizando o BaseValue
		AttributeComponent->SetAttributeBaseValue(AmmoTag, 29.0f);
		AttributeComponent->ConfirmPrediction(AmmoTag, 1);

		// 3. Simula recebimento do pacote de rede no cliente (OnRep simultâneo)
		AttributeComponent->OnRep_ReplicatedAttributes();
		AttributeComponent->OnRep_ConfirmedPredictions();

		// Verificação Jitter-Free
		TestEqual("HUD deve continuar mostrando 29.f de forma estável (sem snapback nem double-dip)", AttributeComponent->GetAttributeValue(AmmoTag), 29.0f);
		TestEqual("Transação correspondente deve ter sido limpa da fila do cliente", AttributeComponent->GetPendingPredictions().Num(), 0);
	});

	It("Cenário 5: Rejeição de Consumo de Atributo e Rollback (Cheat Protection)", [this]()
	{
		APlayerController* LocalPC = TestWorld->SpawnActor<APlayerController>();
		ActiveController = LocalPC;
		ActiveLocalPlayer = NewObject<ULocalPlayer>(GEngine);
		LocalPC->Player = ActiveLocalPlayer;
		APlayerState* LocalPlayerState = TestWorld->SpawnActor<APlayerState>();
		LocalPC->PlayerState = LocalPlayerState;
		LocalPC->SetPawn(TestCharacter);
		TestCharacter->Test_Possess(LocalPC);

		// Configura o Pawn como Cliente Local (Autonomous Proxy) após possessão
		TestCharacter->SetRole(ROLE_AutonomousProxy);

		// 1. Cliente prediz consumo de 1 unidade de munição (ID = 2)
		AttributeComponent->TryConsumeAttribute(AmmoTag, 1.0f, TestCharacter, 2);
		TestEqual("HUD do cliente reduz munição preditivamente", AttributeComponent->GetAttributeValue(AmmoTag), 29.0f);

		// 2. Servidor rejeita (ex: cheat) e envia RPC ClientRollbackPrediction
		AttributeComponent->ClientRollbackPrediction(AmmoTag, 2);
		AttributeComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		// Verificação de Rollback
		TestEqual("HUD deve restaurar munição imediatamente após o rollback do servidor", AttributeComponent->GetAttributeValue(AmmoTag), 30.0f);
		TestEqual("Fila de predições deve ser limpa", AttributeComponent->GetPendingPredictions().Num(), int32(0));
	});
}
