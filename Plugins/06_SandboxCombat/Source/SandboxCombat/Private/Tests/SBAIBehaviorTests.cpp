#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Character/SBCharacter.h"
#include "Components/SBMovementComponent.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBAttributeComponent.h"
#include "GameplayTagsManager.h"
#include "Weapons/SBWeaponBehavior.h"
#include "Weapons/SBWeaponBehaviorHitscan.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "AI/SBAIController.h"
#include "SBGameplayTags.h"
#include "Tests/SBCombatTestHelper.h"
#include "SmartObjectComponent.h"
#include "SmartObjectDefinition.h"
#include "SmartObjectSubsystem.h"
#include "SmartObjectRequestTypes.h"
#include "AI/StateTree/SBStateTreeCombatEvaluator.h"
#include "AI/StateTree/SBStateTreeCombatTasks.h"

BEGIN_DEFINE_SPEC(FSBAIBehaviorTestsSpec, "Sandbox.AIBehavior", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBMovementComponent* MovementComponent;
	USBCombatComponent* CombatComponent;
	USBAttributeComponent* AttrComponent;
	ASBAIController* AIController;

	FGameplayTag StunnedTag;
	FGameplayTag FrozenTag;
	FGameplayTag WeaponTag;
	FGameplayTag HealthTag;
	FGameplayTag MaxHealthTag;
	USBCombatTestHelper* TestHelper;
	UGameInstance* GameInstance;
END_DEFINE_SPEC(FSBAIBehaviorTestsSpec)

void FSBAIBehaviorTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.SetCurrentWorld(TestWorld);

		GameInstance = NewObject<UGameInstance>(GEngine);
		WorldContext.OwningGameInstance = GameInstance;
		GameInstance->InitializeStandalone();
		TestWorld->SetGameInstance(GameInstance);
		if (TestWorld->PersistentLevel)
		{
			TestWorld->PersistentLevel->bIsVisible = true;
		}
		if (AWorldSettings* WS = TestWorld->GetWorldSettings())
		{
			WS->SetRole(ROLE_Authority);
			WS->DefaultGameMode = AGameModeBase::StaticClass();
		}
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		FSBGameplayTags::InitializeNativeTags();
		StunnedTag = FSBGameplayTags::Get().State_Character_Stunned;
		FrozenTag = FSBGameplayTags::Get().State_Character_Frozen;
		WeaponTag = FSBGameplayTags::Get().Combat_Action_Fire;
		HealthTag = FSBGameplayTags::Get().Attribute_Health;
		MaxHealthTag = FSBGameplayTags::Get().Attribute_MaxHealth;

		// Instancia componentes
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		TestCharacter->AddInstanceComponent(StateComponent);
		StateComponent->RegisterComponent();
		
		MovementComponent = NewObject<USBMovementComponent>(TestCharacter);
		TestCharacter->AddInstanceComponent(MovementComponent);
		MovementComponent->RegisterComponent();

		CombatComponent = NewObject<USBCombatComponent>(TestCharacter);
		TestCharacter->AddInstanceComponent(CombatComponent);
		CombatComponent->RegisterComponent();

		AttrComponent = NewObject<USBAttributeComponent>(TestCharacter);
		TestCharacter->AddInstanceComponent(AttrComponent);
		AttrComponent->RegisterComponent();

		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.0f;
		HealthAttr.CurrentValue = 100.0f;
		HealthAttr.MaxValue = 100.0f;
		HealthAttr.MinValue = 0.0f;
		AttrComponent->RegisterAttribute(HealthTag, HealthAttr);
		AttrComponent->RegisterAttribute(MaxHealthTag, HealthAttr);

		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(MovementComponent);
		ISBComponentInterface::Execute_OnInitialize(CombatComponent);
		ISBComponentInterface::Execute_OnInitialize(AttrComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(MovementComponent);
		ISBComponentInterface::Execute_OnReady(CombatComponent);
		ISBComponentInterface::Execute_OnReady(AttrComponent);

		AIController = TestWorld->SpawnActor<ASBAIController>(ASBAIController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestHelper = NewObject<USBCombatTestHelper>();

		FURL URL(TEXT("?game=/Script/Engine.GameModeBase"));
		TestWorld->InitializeActorsForPlay(URL);
		TestWorld->BeginPlay();
	});

	AfterEach([this]()
	{
		if (AIController)
		{
			AIController->Destroy();
			AIController = nullptr;
		}

		if (TestCharacter)
		{
			TestCharacter->Destroy();
			TestCharacter = nullptr;
		}

		if (GameInstance)
		{
			GameInstance->Shutdown();
			GameInstance = nullptr;
		}

		if (TestWorld)
		{
			GEngine->DestroyWorldContext(TestWorld);
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should manage Agro table entries and select the highest agro target correctly via events", [this]()
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASBCharacter* Player1 = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		ASBCharacter* Player2 = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		// Inicialmente não há target de agro
		TestNull("Agro target inicial deve ser nulo", CombatComponent->GetHighestAgroTarget());

		CombatComponent->OnAgroTargetChanged.AddDynamic(TestHelper, &USBCombatTestHelper::HandleAgroTargetChanged);

		// Adiciona agro para Player 1
		CombatComponent->AddAgro(Player1, 50.0f);
		TestEqual("Highest agro deve ser Player 1", CombatComponent->GetHighestAgroTarget(), Cast<APawn>(Player1));
		TestEqual("ReceivedTarget no delegate deve ser Player 1", (APawn*)TestHelper->LastAgroTarget, Cast<APawn>(Player1));

		// Adiciona mais agro para Player 2 (torna Player 2 maior)
		CombatComponent->AddAgro(Player2, 100.0f);
		TestEqual("Highest agro deve ser Player 2", CombatComponent->GetHighestAgroTarget(), Cast<APawn>(Player2));
		TestEqual("ReceivedTarget no delegate deve ser Player 2", (APawn*)TestHelper->LastAgroTarget, Cast<APawn>(Player2));

		// Limpa agro de Player 2
		CombatComponent->ClearAgro(Player2);
		TestEqual("Highest agro deve voltar para Player 1", CombatComponent->GetHighestAgroTarget(), Cast<APawn>(Player1));
		TestEqual("ReceivedTarget no delegate deve ser Player 1", (APawn*)TestHelper->LastAgroTarget, Cast<APawn>(Player1));

		// Destrói Player 1 e verifica que o agro se resolve para nulo e limpa a tabela
		Player1->Destroy();
		TestNull("Highest agro deve ser nulo apos destruicao", CombatComponent->GetHighestAgroTarget());

		Player2->Destroy();
	});

	It("Should override max speed to zero when Stunned or Frozen state tags are applied", [this]()
	{
		// Velocidade padrão configurada
		TestCharacter->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		TestEqual("Velocidade padrao deve ser 600.0", MovementComponent->GetCalculatedMaxSpeed(), 600.0f);

		// Aplica Stun
		StateComponent->AddTag(StunnedTag);
		TestEqual("Velocidade deve cair para zero com StunnedTag", MovementComponent->GetCalculatedMaxSpeed(), 0.0f);

		// Remove Stun
		StateComponent->RemoveTag(StunnedTag);
		TestEqual("Velocidade deve restaurar para 600.0", MovementComponent->GetCalculatedMaxSpeed(), 600.0f);

		// Aplica Frozen
		StateComponent->AddTag(FrozenTag);
		TestEqual("Velocidade deve cair para zero com FrozenTag", MovementComponent->GetCalculatedMaxSpeed(), 0.0f);

		// Remove Frozen
		StateComponent->RemoveTag(FrozenTag);
		TestEqual("Velocidade deve restaurar novamente para 600.0", MovementComponent->GetCalculatedMaxSpeed(), 600.0f);
	});

	It("Should block weapon behavior entry if character has stunned state blocked tag", [this]()
	{
		// Registra o behavior de arma usando a classe concreta USBWeaponBehaviorHitscan e passando a definição
		USBWeaponBehaviorDefinition* Def = NewObject<USBWeaponBehaviorDefinition>();
		Def->BehaviorTag = WeaponTag;
		Def->BlockedTags.AddTag(StunnedTag);
		Def->FireRate = 0.0f;

		USBWeaponBehavior* TestWeapon = NewObject<USBWeaponBehaviorHitscan>(CombatComponent);
		TestWeapon->Initialize(CombatComponent, Def);
		CombatComponent->AddAvailableBehavior(TestWeapon);

		// Ativação sem stun deve passar
		bool bActivated = CombatComponent->RequestWeaponBehavior(WeaponTag);
		TestTrue("Deve ativar arma normalmente", bActivated);
		CombatComponent->StopWeaponBehavior(WeaponTag);

		// Aplica Stun no State Component
		StateComponent->AddTag(StunnedTag);

		// Ativação deve falhar por conta do stun bloqueado
		bool bActivatedBlocked = CombatComponent->RequestWeaponBehavior(WeaponTag);
		TestFalse("Ativacao deve ser bloqueada sob stun", bActivatedBlocked);

		// Remove stun e ativa de novo
		StateComponent->RemoveTag(StunnedTag);
		bool bActivatedAgain = CombatComponent->RequestWeaponBehavior(WeaponTag);
		TestTrue("Ativacao deve voltar a funcionar apos limpar stun", bActivatedAgain);
	});

	It("Should automatically manage AI focus and pause logic under CC", [this]()
	{
		AIController->Possess(TestCharacter);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASBCharacter* Player1 = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		// 1. Adiciona agro -> IA deve focar no Player1
		CombatComponent->AddAgro(Player1, 50.0f);
		TestEqual("AI focus deve ser Player 1", AIController->GetFocusActor(), Cast<AActor>(Player1));

		// 2. Aplica Stun -> IA deve limpar foco e parar
		StateComponent->AddTag(StunnedTag);
		TestNull("AI focus deve ser limpo sob stun", AIController->GetFocusActor());

		// 3. Limpa Stun -> IA deve recuperar o foco no Player1
		StateComponent->RemoveTag(StunnedTag);
		TestEqual("AI focus deve restaurar para Player 1", AIController->GetFocusActor(), Cast<AActor>(Player1));

		Player1->Destroy();
	});

	It("Should transition boss phases based on HP thresholds", [this]()
	{
		AIController->BossPhaseHPThresholds.Add(0.75f);
		AIController->BossPhaseHPThresholds.Add(0.50f);
		AIController->BossPhaseHPThresholds.Add(0.25f);

		AIController->Possess(TestCharacter);

		TestEqual("Boss deve iniciar na fase 0", AIController->GetCurrentBossPhase(), 0);

		AIController->OnBossPhaseChanged.AddDynamic(TestHelper, &USBCombatTestHelper::HandleBossPhaseChanged);

		// Vida cai para 70% (Cruza limiar de 75%)
		AttrComponent->SetAttributeBaseValue(HealthTag, 70.0f);
		TestEqual("Fase atual do boss deve ser 1", AIController->GetCurrentBossPhase(), 1);
		TestEqual("Delegate OnBossPhaseChanged deve receber fase 1", TestHelper->LastBossPhase, 1);

		// Vida cai para 40% (Cruza limiar de 50%)
		AttrComponent->SetAttributeBaseValue(HealthTag, 40.0f);
		TestEqual("Fase atual do boss deve ser 2", AIController->GetCurrentBossPhase(), 2);
		TestEqual("Delegate OnBossPhaseChanged deve receber fase 2", TestHelper->LastBossPhase, 2);

		// Vida sobe e desce sem cruzar novos thresholds
		AttrComponent->SetAttributeBaseValue(HealthTag, 45.0f);
		AttrComponent->SetAttributeBaseValue(HealthTag, 42.0f);
		TestEqual("Fase atual do boss deve continuar 2", AIController->GetCurrentBossPhase(), 2);
	});

	It("Should find, claim, and release smart object slots", [this]()
	{
		// 1. Cria definição e componente de Smart Object
		USmartObjectDefinition* SODef = NewObject<USmartObjectDefinition>(TestWorld);
		
		FGameplayTag ActivityTag = FSBGameplayTags::Get().Activity_TestInteraction;
		SODef->SetActivityTags(FGameplayTagContainer(ActivityTag));
		
		FSmartObjectSlotDefinition& SlotDef = SODef->DebugAddSlot();
		SlotDef.ActivityTags.AddTag(ActivityTag);
		
		USBTestSmartObjectBehaviorDefinition* BehaviorDef = NewObject<USBTestSmartObjectBehaviorDefinition>(SODef);
		SlotDef.BehaviorDefinitions.Add(BehaviorDef);

		AActor* SOActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
		USceneComponent* SOActorRoot = NewObject<USceneComponent>(SOActor, TEXT("SOActorRoot"));
		SOActor->SetRootComponent(SOActorRoot);
		SOActor->AddInstanceComponent(SOActorRoot);
		SOActorRoot->RegisterComponent();
		SOActor->SetActorLocation(FVector(100.0f, 0.0f, 0.0f));

		USmartObjectComponent* SOComp = NewObject<USmartObjectComponent>(SOActor);
		SOComp->SetDefinition(SODef);
		SOActor->AddInstanceComponent(SOComp);
		SOComp->RegisterComponent();

		USmartObjectSubsystem* SOSubsystem = USmartObjectSubsystem::GetCurrent(TestWorld);
		if (SOSubsystem)
		{
			SOSubsystem->RegisterSmartObject(SOComp);
		}

		// Possui o character no AIController
		AIController->Possess(TestCharacter);

		// 2. Busca o Smart Object próximo
		TArray<FSmartObjectRequestResult> SearchResults;
		FGameplayTagQuery FilterQuery = FGameplayTagQuery::MakeQuery_MatchAnyTags(FGameplayTagContainer(ActivityTag));
		
		bool bFound = AIController->FindNearbySmartObjects(SearchResults, FilterQuery, 500.0f);
		TestTrue("Should find nearby Smart Object matching activity tag", bFound);
		TestTrue("SearchResults should not be empty", SearchResults.Num() > 0);

		if (SearchResults.Num() > 0)
		{
			// 3. Reivindica (claim) o slot
			FSmartObjectClaimHandle ClaimHandle;
			bool bClaimed = AIController->ClaimSmartObjectSlot(SearchResults[0], ClaimHandle);
			TestTrue("Should successfully claim the Smart Object slot", bClaimed);
			TestTrue("Claim handle should be valid", ClaimHandle.IsValid());

			if (ClaimHandle.IsValid())
			{
				// 4. Obtém o transform do slot
				FTransform SlotTransform;
				bool bGotTransform = AIController->GetSmartObjectSlotTransform(ClaimHandle, SlotTransform);
				TestTrue("Should retrieve slot transform", bGotTransform);
				
				// 5. Libera o slot
				bool bReleased = AIController->ReleaseSmartObjectSlot(ClaimHandle);
				TestTrue("Should release claimed slot", bReleased);
			}
		}

		SOActor->Destroy();
	});

	It("Should correctly populate StateTree Combat Evaluator instance data from character state and attributes", [this]()
	{
		AIController->Possess(TestCharacter);

		FSBStateTreeCombatEvaluator Evaluator;
		TestEqual("Instance data type must match", (const UStruct*)Evaluator.GetInstanceDataType(), (const UStruct*)FSBStateTreeCombatEvaluatorInstanceData::StaticStruct());

		FSBStateTreeCombatEvaluatorInstanceData InstanceData;
		InstanceData.AIController = AIController;

		// 1. Initial State: Health 100%, No Stun, No Target
		AttrComponent->SetAttributeBaseValue(HealthTag, 100.0f);
		TestCharacter->SetActorLocation(FVector::ZeroVector);

		float Health = AttrComponent->GetAttributeValue(HealthTag);
		float MaxHealth = AttrComponent->GetAttributeValue(MaxHealthTag);
		InstanceData.HealthRatio = FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f);
		InstanceData.bIsStunned = StateComponent->HasTag(StunnedTag);
		InstanceData.bIsDead = (Health <= 0.0f);
		InstanceData.TargetActor = CombatComponent->GetHighestAgroTarget();

		TestEqual("Health ratio should be 1.0", InstanceData.HealthRatio, 1.0f);
		TestFalse("Should not be stunned", InstanceData.bIsStunned);
		TestFalse("Should not be dead", InstanceData.bIsDead);
		TestNull("Should have no target", InstanceData.TargetActor.Get());

		// 2. Modified State: Damaged & Stunned & Target
		AttrComponent->SetAttributeBaseValue(HealthTag, 25.0f);
		StateComponent->AddTag(StunnedTag);
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASBCharacter* TargetChar = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector(300.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		CombatComponent->AddAgro(TargetChar, 100.0f);

		Health = AttrComponent->GetAttributeValue(HealthTag);
		MaxHealth = AttrComponent->GetAttributeValue(MaxHealthTag);
		InstanceData.HealthRatio = FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f);
		InstanceData.bIsStunned = StateComponent->HasTag(StunnedTag);
		InstanceData.bIsDead = (Health <= 0.0f);
		InstanceData.TargetActor = CombatComponent->GetHighestAgroTarget();

		TestEqual("Health ratio should be 0.25", InstanceData.HealthRatio, 0.25f);
		TestTrue("Should be stunned", InstanceData.bIsStunned);
		TestFalse("Should not be dead", InstanceData.bIsDead);
		TestEqual("Target actor should match highest agro", InstanceData.TargetActor.Get(), Cast<AActor>(TargetChar));

		TargetChar->Destroy();
	});

	It("Should validate StateTree MoveAndAttack task execution conditions and range checks", [this]()
	{
		AIController->Possess(TestCharacter);

		FSBStateTreeTask_MoveAndAttack Task;
		TestEqual("Task instance data type must match", (const UStruct*)Task.GetInstanceDataType(), (const UStruct*)FSBStateTreeTask_MoveAndAttackInstanceData::StaticStruct());

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASBCharacter* TargetChar = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);

		FSBStateTreeTask_MoveAndAttackInstanceData TaskData;
		TaskData.AIController = AIController;
		TaskData.TargetActor = TargetChar;
		TaskData.AttackRange = 200.0f;
		TaskData.ActionTag = WeaponTag;

		// Distance is 100 <= AttackRange 200 -> should trigger attack immediately
		float Distance = FVector::Dist(TestCharacter->GetActorLocation(), TargetChar->GetActorLocation());
		TestTrue("Target is within attack range", Distance <= TaskData.AttackRange);

		CombatComponent->RequestWeaponBehavior(WeaponTag);
		TaskData.bAttackTriggered = true;
		TestTrue("Attack should be triggered", TaskData.bAttackTriggered);

		TargetChar->Destroy();
	});
}
