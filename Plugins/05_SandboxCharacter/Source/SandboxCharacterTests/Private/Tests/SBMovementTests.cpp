#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBMovementComponent.h"
#include "Character/SBCharacter.h"
#include "Movement/Behaviors/SBMovementBehavior.h"
#include "Movement/Aggregator/SBMovementModifierAggregator.h"
#include "Movement/Behaviors/SBMovementBehaviorCrouch.h"
#include "Movement/Behaviors/SBMovementBehaviorSprint.h"
#include "Movement/DataAssets/SBMovementConfigDataAsset.h"
#include "Movement/DataAssets/SBMovementBehaviorDefinition.h"
#include "GameplayTagsManager.h"

BEGIN_DEFINE_SPEC(FSBMovementTestsSpec, "Sandbox.Character.Movement", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBMovementComponent* MovementComponent;
	USBMovementConfigDataAsset* ConfigAsset;
	
	// Tags de teste
	FGameplayTag CrouchTag;
	FGameplayTag SprintTag;
	FGameplayTag ReentrantTag;
END_DEFINE_SPEC(FSBMovementTestsSpec)

void FSBMovementTestsSpec::Define()
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
		
		// Instancia e registra os componentes
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();
		
		MovementComponent = NewObject<USBMovementComponent>(TestCharacter);
		MovementComponent->RegisterComponent();

		// Inicialização explícita no ciclo do teste
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(MovementComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(MovementComponent);

		// Inicializa tags úteis
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		CrouchTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Crouching"));
		SprintTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Sprinting"));
		ReentrantTag = TagsManager.AddNativeGameplayTag(TEXT("State.Character.Reentrant"));
		TagsManager.AddNativeGameplayTag(TEXT("Movement.Group.Stance"));
		TagsManager.AddNativeGameplayTag(TEXT("Stat.MovementSpeed"));

		// Cria definições e data assets em memória para configurar Crouch e Sprint
		ConfigAsset = NewObject<USBMovementConfigDataAsset>();

		USBMovementBehaviorCrouchDefinition* CrouchDef = NewObject<USBMovementBehaviorCrouchDefinition>();
		CrouchDef->BehaviorTag = CrouchTag;
		CrouchDef->StackPriority = 20;
		CrouchDef->ExclusivityGroup = FGameplayTag::RequestGameplayTag(TEXT("Movement.Group.Stance"));
		CrouchDef->BlockedTags.AddTag(SprintTag); // Se Sprint estiver ativo, bloqueia Crouch
		
		FSBModifierEntry CrouchMod;
		CrouchMod.TargetStatTag = FGameplayTag::RequestGameplayTag(TEXT("Stat.MovementSpeed"));
		CrouchMod.Operation = ESBModifierOperation::Override;
		CrouchMod.Value = 300.f; // Reduz velocidade
		CrouchDef->MovementModifiers.Add(CrouchMod);

		USBMovementBehaviorDefinition* SprintDef = NewObject<USBMovementBehaviorDefinition>();
		SprintDef->BehaviorTag = SprintTag;
		SprintDef->StackPriority = 50;
		SprintDef->ExclusivityGroup = FGameplayTag::RequestGameplayTag(TEXT("Movement.Group.Stance"));
		
		FSBModifierEntry SprintMod;
		SprintMod.TargetStatTag = FGameplayTag::RequestGameplayTag(TEXT("Stat.MovementSpeed"));
		SprintMod.Operation = ESBModifierOperation::Multiply;
		SprintMod.Value = 1.5f; // Multiplica velocidade
		SprintDef->MovementModifiers.Add(SprintMod);

		USBMovementBehaviorDefinition* ReentrantDef = NewObject<USBMovementBehaviorDefinition>();
		ReentrantDef->BehaviorTag = ReentrantTag;
		ReentrantDef->StackPriority = 99;

		FSBMovementBehaviorConfigEntry CrouchEntry;
		CrouchEntry.BehaviorClass = USBMovementBehaviorCrouch::StaticClass();
		CrouchEntry.DefinitionAsset = CrouchDef;
		ConfigAsset->ConfiguredBehaviors.Add(CrouchEntry);

		FSBMovementBehaviorConfigEntry SprintEntry;
		SprintEntry.BehaviorClass = USBMovementBehaviorSprint::StaticClass();
		SprintEntry.DefinitionAsset = SprintDef;
		ConfigAsset->ConfiguredBehaviors.Add(SprintEntry);

		FSBMovementBehaviorConfigEntry ReentrantEntry;
		ReentrantEntry.BehaviorClass = USBMockReentrantBehavior::StaticClass();
		ReentrantEntry.DefinitionAsset = ReentrantDef;
		ConfigAsset->ConfiguredBehaviors.Add(ReentrantEntry);

		MovementComponent->LoadMovementConfig(ConfigAsset);
	});

	AfterEach([this]()
	{
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

	It("Cenário 1: Entrada e Saída voluntária de comportamento", [this]()
	{
		// 1. Tenta ativar Crouch
		bool bSuccess = MovementComponent->RequestBehavior(CrouchTag);
		TestTrue("Crouch deve entrar com sucesso", bSuccess);
		TestTrue("Tag de agachamento deve estar ativa no StateComponent", StateComponent->HasTag(CrouchTag));
		TestEqual("Aggregator deve aplicar override do Crouch", MovementComponent->GetSpeedModifierAggregator()->CalculateFinalValue(600.f), 300.f);

		// 2. Tenta parar Crouch
		MovementComponent->StopBehavior(CrouchTag);
		MovementComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		TestFalse("Tag de agachamento deve ser limpa após a saída", StateComponent->HasTag(CrouchTag));
		TestEqual("Aggregator deve retornar ao valor base de velocidade", MovementComponent->GetSpeedModifierAggregator()->CalculateFinalValue(600.f), 600.f);
	});

	It("Cenário 2: Ejeção por ExclusivityGroup e limpeza do Aggregator", [this]()
	{
		// 1. Ativa o Crouch (Grupo: Stance, Prioridade baixa)
		MovementComponent->RequestBehavior(CrouchTag);
		TestTrue("Crouch ativo inicialmente", StateComponent->HasTag(CrouchTag));
		TestEqual("Velocidade com Crouch ativo", MovementComponent->GetSpeedModifierAggregator()->CalculateFinalValue(600.f), 300.f);

		// 2. Ativa o Sprint (Grupo: Stance, Prioridade alta, ejeta o Crouch)
		MovementComponent->RequestBehavior(SprintTag);
		MovementComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

		TestFalse("Crouch deve ser ejetado devido ao ExclusivityGroup compartilhado", StateComponent->HasTag(CrouchTag));
		TestTrue("Sprint deve assumir a pilha", StateComponent->HasTag(SprintTag));
		// Sprint é um multiplicador de 1.5x. O Crouch (override 300) foi limpo, então deve ser 600 * 1.5 = 900
		TestEqual("Velocidade final deve ter apenas o multiplicador do Sprint", MovementComponent->GetSpeedModifierAggregator()->CalculateFinalValue(600.f), 900.f);
	});

	It("Cenário 3: Precedência Assimétrica por BlockedTags", [this]()
	{
		// 1. Ativa o Sprint
		MovementComponent->RequestBehavior(SprintTag);
		TestTrue("Sprint ativo", StateComponent->HasTag(SprintTag));

		// 2. Tenta ativar o Crouch (Bloqueado por BlockedTags do Crouch contra SprintTag)
		bool bCrouchSuccess = MovementComponent->RequestBehavior(CrouchTag);
		
		TestFalse("Crouch deve falhar ao tentar entrar enquanto Sprint está ativo", bCrouchSuccess);
		TestFalse("Tag de Crouch não deve estar presente no StateComponent", StateComponent->HasTag(CrouchTag));
		TestTrue("Sprint deve permanecer inalterado na pilha", StateComponent->HasTag(SprintTag));
		TestEqual("Velocidade final deve permanecer 900 (apenas Sprint)", MovementComponent->GetSpeedModifierAggregator()->CalculateFinalValue(600.f), 900.f);
	});

	It("Cenário 4: Reentrância Recursiva de Pilha (FSBStackMutationGuard)", [this]()
	{
		// 1. Busca a instância do mock do registry que foi carregada no LoadMovementConfig
		USBMockReentrantBehavior* MockReentrant = Cast<USBMockReentrantBehavior>(MovementComponent->FindAvailableBehaviorByTag(ReentrantTag));
		
		TestNotNull("O mock behavior deve ter sido instanciado", MockReentrant);
		if (MockReentrant)
		{
			MockReentrant->OwnerMC = MovementComponent;
			MockReentrant->TagToRequestOnExit = SprintTag;

			// Ativa o mock reentrant
			MovementComponent->RequestBehavior(ReentrantTag);
			TestTrue("Mock reentrant deve estar ativo", MovementComponent->HasBehavior(ReentrantTag));

			// Para o mock reentrant, o que disparará Exit_Implementation, que tenta reentrar chamando RequestBehavior(SprintTag)
			MovementComponent->StopBehavior(ReentrantTag);

			// Executa tick para resolver a mutação e filas deferidas
			MovementComponent->TickComponent(0.016f, LEVELTICK_All, nullptr);

			// Se o teste chegou aqui sem crashar (iterator invalidation), a proteção RAII funcionou
			TestTrue("Sprint deve ter sido ativado após a saída reentrante", MovementComponent->HasBehavior(SprintTag));
			TestEqual("Velocidade final após reentrância resolvida", MovementComponent->GetSpeedModifierAggregator()->CalculateFinalValue(600.f), 900.f);
		}
	});
}
