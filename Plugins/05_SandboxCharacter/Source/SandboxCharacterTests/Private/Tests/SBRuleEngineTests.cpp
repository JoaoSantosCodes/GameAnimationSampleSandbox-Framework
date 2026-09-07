#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Subsystems/SBSandboxRuleSubsystem.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBRuleEngineTestsSpec, "Sandbox.RuleEngine", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	UGameInstance* GameInstance;
	ASBCharacter* TestCharacter;
	USBAttributeComponent* AttrComponent;
	USBStateComponent* StateComponent;
	USBSandboxRuleSubsystem* RuleSubsystem;

	FGameplayTag HealthTag;
	FGameplayTag StunnedTag;
	FGameplayTag DeadTag;
END_DEFINE_SPEC(FSBRuleEngineTestsSpec)

void FSBRuleEngineTestsSpec::Define()
{
	BeforeEach([this]()
	{
		GameInstance = NewObject<UGameInstance>(GEngine);
		GameInstance->InitializeStandalone();

		TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
		TestWorld->SetGameInstance(GameInstance);
		RuleSubsystem = GameInstance->GetSubsystem<USBSandboxRuleSubsystem>();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		FSBGameplayTags::InitializeNativeTags();
		HealthTag = FSBGameplayTags::Get().Attribute_Health;
		StunnedTag = FSBGameplayTags::Get().State_Character_Stunned;
		DeadTag = FSBGameplayTags::Get().State_Character_Dead;

		// Instancia componentes
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		TestCharacter->AddInstanceComponent(StateComponent);
		StateComponent->RegisterComponent();

		AttrComponent = NewObject<USBAttributeComponent>(TestCharacter);
		TestCharacter->AddInstanceComponent(AttrComponent);
		AttrComponent->RegisterComponent();

		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.0f;
		HealthAttr.CurrentValue = 100.0f;
		HealthAttr.MaxValue = 100.0f;
		HealthAttr.MinValue = 0.0f;
		AttrComponent->RegisterAttribute(HealthTag, HealthAttr);

		// Inicializa e ativa os componentes
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(AttrComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(AttrComponent);
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

		if (GameInstance)
		{
			GameInstance->Shutdown();
			GameInstance = nullptr;
		}
	});

	It("Should evaluate numeric attribute conditions correctly", [this]()
	{
		// Regra: Vida >= 50
		FSBRuleCondition CondGreater;
		CondGreater.ConditionTag = HealthTag;
		CondGreater.Operator = ESBRuleOperator::GreaterThanOrEqual;
		CondGreater.NumericValue = 50.0f;

		FSBRule RuleGreater;
		RuleGreater.Conditions.Add(CondGreater);

		TestTrue("Health 100 should be >= 50", RuleSubsystem->EvaluateRule(RuleGreater, TestCharacter));

		// Modifica vida para 30
		AttrComponent->SetAttributeBaseValue(HealthTag, 30.0f);
		TestFalse("Health 30 should not be >= 50", RuleSubsystem->EvaluateRule(RuleGreater, TestCharacter));

		// Regra: Vida < 40
		FSBRuleCondition CondLess;
		CondLess.ConditionTag = HealthTag;
		CondLess.Operator = ESBRuleOperator::LessThan;
		CondLess.NumericValue = 40.0f;

		FSBRule RuleLess;
		RuleLess.Conditions.Add(CondLess);

		TestTrue("Health 30 should be < 40", RuleSubsystem->EvaluateRule(RuleLess, TestCharacter));
	});

	It("Should evaluate state tag conditions correctly", [this]()
	{
		// Regra: Possui tag Stunned
		FSBRuleCondition CondHasStun;
		CondHasStun.Operator = ESBRuleOperator::HasTag;
		CondHasStun.TagValue = StunnedTag;

		FSBRule RuleStun;
		RuleStun.Conditions.Add(CondHasStun);

		// Inicialmente falso
		TestFalse("Should not have Stunned tag initially", RuleSubsystem->EvaluateRule(RuleStun, TestCharacter));

		// Adiciona a tag Stunned
		StateComponent->AddTag(StunnedTag);
		TestTrue("Should have Stunned tag after addition", RuleSubsystem->EvaluateRule(RuleStun, TestCharacter));

		// Regra: Nao possui tag Dead
		FSBRuleCondition CondNotDead;
		CondNotDead.Operator = ESBRuleOperator::DoesNotHaveTag;
		CondNotDead.TagValue = DeadTag;

		FSBRule RuleNotDead;
		RuleNotDead.Conditions.Add(CondNotDead);

		TestTrue("Should evaluate true for NOT having Dead tag", RuleSubsystem->EvaluateRule(RuleNotDead, TestCharacter));

		// Adiciona tag Dead
		StateComponent->AddTag(DeadTag);
		TestFalse("Should evaluate false for NOT having Dead tag after adding Dead", RuleSubsystem->EvaluateRule(RuleNotDead, TestCharacter));
	});

	It("Should evaluate combined AND and OR logics correctly", [this]()
	{
		// Regra AND: Vida >= 50 E Possui tag Stunned
		FSBRule RuleAnd;
		RuleAnd.bRequireAll = true;

		FSBRuleCondition CondHP;
		CondHP.ConditionTag = HealthTag;
		CondHP.Operator = ESBRuleOperator::GreaterThanOrEqual;
		CondHP.NumericValue = 50.0f;
		RuleAnd.Conditions.Add(CondHP);

		FSBRuleCondition CondStun;
		CondStun.Operator = ESBRuleOperator::HasTag;
		CondStun.TagValue = StunnedTag;
		RuleAnd.Conditions.Add(CondStun);

		// Inicialmente deve falhar (não possui Stunned)
		TestFalse("AND should fail because Stunned tag is missing", RuleSubsystem->EvaluateRule(RuleAnd, TestCharacter));

		// Adiciona tag Stunned -> deve passar
		StateComponent->AddTag(StunnedTag);
		TestTrue("AND should pass when both HP >= 50 and Stunned tag exist", RuleSubsystem->EvaluateRule(RuleAnd, TestCharacter));

		// Modifica vida para 30 -> deve falhar
		AttrComponent->SetAttributeBaseValue(HealthTag, 30.0f);
		TestFalse("AND should fail when HP falls below 50", RuleSubsystem->EvaluateRule(RuleAnd, TestCharacter));

		// Regra OR: Vida >= 50 OU Possui tag Stunned
		FSBRule RuleOr;
		RuleOr.bRequireAll = false;
		RuleOr.Conditions.Add(CondHP);
		RuleOr.Conditions.Add(CondStun);

		// Vida esta em 30, mas possui Stunned -> deve passar
		TestTrue("OR should pass because Stunned tag is present", RuleSubsystem->EvaluateRule(RuleOr, TestCharacter));

		// Remove tag Stunned -> deve falhar (ambos falsos)
		StateComponent->RemoveTag(StunnedTag);
		TestFalse("OR should fail when both HP is low and Stunned is absent", RuleSubsystem->EvaluateRule(RuleOr, TestCharacter));
	});
}
