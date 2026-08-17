#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBAbilityComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Abilities/SBAbility.h"
#include "Behaviors/SBGameplayBehaviorDefinition.h"
#include "Input/SBInputComponent.h"
#include "Input/SBInputConfig.h"
#include "GameplayTagsManager.h"

BEGIN_DEFINE_SPEC(FSBAbilityTestsSpec, "Sandbox.Character.Abilities", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBAbilityComponent* AbilityComponent;
	USBAttributeComponent* AttributeComponent;

	FGameplayTag AbilityTag;
	FGameplayTag ManaTag;
	FGameplayTag BlockTag;
END_DEFINE_SPEC(FSBAbilityTestsSpec)

void FSBAbilityTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		TestCharacter->SetRole(ROLE_Authority);

		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();

		AbilityComponent = NewObject<USBAbilityComponent>(TestCharacter);
		AbilityComponent->RegisterComponent();

		AttributeComponent = NewObject<USBAttributeComponent>(TestCharacter);
		AttributeComponent->RegisterComponent();

		// Inicialização
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(AbilityComponent);
		ISBComponentInterface::Execute_OnInitialize(AttributeComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(AbilityComponent);
		ISBComponentInterface::Execute_OnReady(AttributeComponent);

		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		AbilityTag = TagsManager.AddNativeGameplayTag(FName(TEXT("Ability.Test")));
		ManaTag = TagsManager.AddNativeGameplayTag(FName(TEXT("Attribute.Mana")));
		BlockTag = TagsManager.AddNativeGameplayTag(FName(TEXT("State.Blocked")));
		TagsManager.AddNativeGameplayTag(FName(TEXT("Input.Action.Ability1")));
		TagsManager.AddNativeGameplayTag(FName(TEXT("State.Cooldown.Ability.Fire")));

		FSBAttribute InitialMana;
		InitialMana.BaseValue = 100.0f;
		InitialMana.CurrentValue = 100.0f;
		InitialMana.MinValue = 0.0f;
		InitialMana.MaxValue = 100.0f;
		AttributeComponent->RegisterAttribute(ManaTag, InitialMana);
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(true);
		}
	});

	It("Cenario_1_Ativacao_e_Cooldown_Replicado", [this]()
	{
		AttributeComponent->SetAttributeBaseValue(ManaTag, 100.0f);

		USBAbility* TestAbility = NewObject<USBAbility>(TestCharacter, USBAbility::StaticClass());
		TestAbility->AbilityTag = AbilityTag;
		TestAbility->CooldownDuration = 3.0f;

		USBGameplayBehaviorDefinition* Def = NewObject<USBGameplayBehaviorDefinition>(TestCharacter);
		Def->BehaviorTag = AbilityTag;
		Def->StackPriority = 10;
		TestAbility->Initialize(AbilityComponent, Def);

		AbilityComponent->AddAvailableBehavior(TestAbility);

		bool bActivated = AbilityComponent->ActivateAbilityByTag(AbilityTag);
		TestTrue("Habilidade deve ativar com sucesso", bActivated);
		TestTrue("Habilidade deve constar como ativa na pilha", AbilityComponent->HasBehavior(AbilityTag));

		TestTrue("Habilidade deve estar em cooldown", AbilityComponent->IsAbilityOnCooldown(AbilityTag));
		TestTrue("Tempo restante do cooldown deve ser maior que zero", AbilityComponent->GetRemainingCooldownTime(AbilityTag) > 0.0f);

		bool bReactivated = AbilityComponent->ActivateAbilityByTag(AbilityTag);
		TestFalse("Ativacao repetida em cooldown deve falhar", bReactivated);
	});

	It("Cenario_2_Consumo_e_Rollback_Transacional", [this]()
	{
		AttributeComponent->SetAttributeBaseValue(ManaTag, 100.0f);

		USBAbility* TestAbility = NewObject<USBAbility>(TestCharacter, USBAbility::StaticClass());
		TestAbility->AbilityTag = AbilityTag;
		TestAbility->ResourceTag = ManaTag;
		TestAbility->ResourceCost = 60.0f;
		TestAbility->CooldownDuration = 0.0f;

		USBGameplayBehaviorDefinition* Def = NewObject<USBGameplayBehaviorDefinition>(TestCharacter);
		Def->BehaviorTag = AbilityTag;
		TestAbility->Initialize(AbilityComponent, Def);

		AbilityComponent->AddAvailableBehavior(TestAbility);

		bool bActivated = AbilityComponent->ActivateAbilityByTag(AbilityTag);
		TestTrue("Primeira ativacao deve passar", bActivated);
		
		float CurrentMana = AttributeComponent->GetAttributeValue(ManaTag);
		TestEqual("Mana restante deve ser 40.0", CurrentMana, 40.0f);

		bool bInsuficiente = AbilityComponent->ActivateAbilityByTag(AbilityTag);
		TestFalse("Ativacao sem mana deve falhar", bInsuficiente);
		
		float PostMana = AttributeComponent->GetAttributeValue(ManaTag);
		TestEqual("Mana nao deve ter sido alterada apos falha", PostMana, 40.0f);
	});

	It("Cenario_3_Enhanced_Input_Mapping", [this]()
	{
		USBAbility* TestAbility = NewObject<USBAbility>(TestCharacter, USBAbility::StaticClass());
		TestAbility->AbilityTag = AbilityTag;
		TestAbility->CooldownDuration = 0.0f;

		USBGameplayBehaviorDefinition* Def = NewObject<USBGameplayBehaviorDefinition>(TestCharacter);
		Def->BehaviorTag = AbilityTag;
		TestAbility->Initialize(AbilityComponent, Def);

		AbilityComponent->AddAvailableBehavior(TestAbility);

		FGameplayTag InputTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Action.Ability1"));
		AbilityComponent->InputToAbilityMap.Add(InputTag, AbilityTag);

		TestFalse("Habilidade deve estar inativa inicialmente", AbilityComponent->HasBehavior(AbilityTag));

		AbilityComponent->Input_AbilityInputPressed(InputTag);
		TestTrue("Habilidade deve ativar pelo trigger de press", AbilityComponent->HasBehavior(AbilityTag));

		AbilityComponent->Input_AbilityInputReleased(InputTag);
		TestFalse("Habilidade deve desativar pelo trigger de release", AbilityComponent->HasBehavior(AbilityTag));
	});

	It("Should apply dynamic CooldownTag on activation and remove on expiry", [this]()
	{
		FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(TEXT("State.Cooldown.Ability.Fire"));

		USBAbility* TestAbility = NewObject<USBAbility>(TestCharacter, USBAbility::StaticClass());
		TestAbility->AbilityTag = AbilityTag;
		TestAbility->CooldownDuration = 2.0f;
		TestAbility->CooldownTag = CooldownTag;

		USBGameplayBehaviorDefinition* Def = NewObject<USBGameplayBehaviorDefinition>(TestCharacter);
		Def->BehaviorTag = AbilityTag;
		TestAbility->Initialize(AbilityComponent, Def);
		AbilityComponent->AddAvailableBehavior(TestAbility);

		// Inicialmente tempo é 0
		TestWorld->TimeSeconds = 0.0f;

		// Ativação deve aplicar a tag de cooldown
		bool bActivated = AbilityComponent->ActivateAbilityByTag(AbilityTag);
		TestTrue("Ativacao deve passar", bActivated);
		TestTrue("Deve possuir a tag de cooldown", StateComponent->HasTag(CooldownTag));

		// Tick passados 1.0s (menor que CooldownDuration) -> deve continuar em cooldown
		TestWorld->TimeSeconds = 1.0f;
		AbilityComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);
		TestTrue("Deve continuar com a tag de cooldown", StateComponent->HasTag(CooldownTag));

		// Tick passados mais 1.0s (total 2.0s) -> deve expirar o cooldown
		TestWorld->TimeSeconds = 2.0f;
		AbilityComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);
		TestFalse("Deve ter removido a tag de cooldown", StateComponent->HasTag(CooldownTag));
	});

	It("Should support passive Mana regeneration and regen delay", [this]()
	{
		// Registra 100 de mana
		AttributeComponent->SetAttributeBaseValue(ManaTag, 100.0f);

		// Configura custo na habilidade
		USBAbility* TestAbility = NewObject<USBAbility>(TestCharacter, USBAbility::StaticClass());
		TestAbility->AbilityTag = AbilityTag;
		TestAbility->ResourceTag = ManaTag;
		TestAbility->ResourceCost = 40.0f;

		USBGameplayBehaviorDefinition* Def = NewObject<USBGameplayBehaviorDefinition>(TestCharacter);
		Def->BehaviorTag = AbilityTag;
		TestAbility->Initialize(AbilityComponent, Def);
		AbilityComponent->AddAvailableBehavior(TestAbility);

		TestWorld->TimeSeconds = 0.0f;

		// Ativa para consumir mana
		bool bActivated = AbilityComponent->ActivateAbilityByTag(AbilityTag);
		TestTrue("Ativacao deve consumir mana", bActivated);
		TestEqual("Mana deve cair para 60", AttributeComponent->GetAttributeValue(ManaTag), 60.0f);

		// Tick 1.9s -> Dentro do delay de 2.0s, então não deve regenerar mana
		TestWorld->TimeSeconds = 1.9f;
		AbilityComponent->TickComponent(1.9f, LEVELTICK_All, nullptr);
		TestEqual("Mana nao deve regenerar durante delay", AttributeComponent->GetAttributeValue(ManaTag), 60.0f);

		// Tick mais 1.0s -> Total 2.9s (passou do delay de 2.0s). Deve regenerar 1.0s de mana.
		// Taxa = 5.f/s. 1.0s * 5 = 5.0f de mana regenerada. Total = 65.0f.
		TestWorld->TimeSeconds = 2.9f;
		AbilityComponent->TickComponent(1.0f, LEVELTICK_All, nullptr);
		TestEqual("Mana deve comecar a regenerar apos o delay", AttributeComponent->GetAttributeValue(ManaTag), 65.0f);
	});

	It("Should clean up CooldownTag and cooldown list entry on ClientRollbackAbility", [this]()
	{
		FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(TEXT("State.Cooldown.Ability.Fire"));

		USBAbility* TestAbility = NewObject<USBAbility>(TestCharacter, USBAbility::StaticClass());
		TestAbility->AbilityTag = AbilityTag;
		TestAbility->CooldownDuration = 5.0f;
		TestAbility->CooldownTag = CooldownTag;
		TestAbility->ResourceTag = ManaTag;
		TestAbility->ResourceCost = 30.0f;

		USBGameplayBehaviorDefinition* Def = NewObject<USBGameplayBehaviorDefinition>(TestCharacter);
		Def->BehaviorTag = AbilityTag;
		TestAbility->Initialize(AbilityComponent, Def);
		AbilityComponent->AddAvailableBehavior(TestAbility);

		// Configura o papel como AutonomousProxy para forçar simulação de predição cliente
		TestCharacter->SetRole(ROLE_AutonomousProxy);

		// Ativa localmente (simula cliente predizendo consumo e cooldown)
		bool bActivated = AbilityComponent->ActivateAbilityByTag(AbilityTag);
		TestTrue("Cliente ativou habilidade", bActivated);
		TestEqual("Mana consumida", AttributeComponent->GetAttributeValue(ManaTag), 70.0f);
		TestTrue("Tag de cooldown aplicada", StateComponent->HasTag(CooldownTag));

		// Servidor rejeita a ativação e executa rollback (PredictionId = 1)
		AbilityComponent->ClientRollbackAbility(AbilityTag, 1);

		// Todos os efeitos do cooldown e consumo devem ser cancelados e a mana restaurada
		TestFalse("Tag de cooldown deve ser removida no rollback", StateComponent->HasTag(CooldownTag));
		TestFalse("Habilidade nao deve constar em cooldown", AbilityComponent->IsAbilityOnCooldown(AbilityTag));
		TestEqual("Mana restaurada para o estado original", AttributeComponent->GetAttributeValue(ManaTag), 100.0f);
	});
}
