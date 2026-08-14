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

		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();

		AbilityComponent = NewObject<USBAbilityComponent>(TestCharacter);
		AbilityComponent->RegisterComponent();

		AttributeComponent = NewObject<USBAttributeComponent>(TestCharacter);
		AttributeComponent->RegisterComponent();

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
}
