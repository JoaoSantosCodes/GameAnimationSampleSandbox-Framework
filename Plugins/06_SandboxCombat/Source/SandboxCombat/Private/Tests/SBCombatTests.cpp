#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Weapons/SBWeaponBehavior.h"
#include "Weapons/SBWeaponBehaviorHitscan.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "GameplayTagsManager.h"
#include "Character/SBCharacter.h"

BEGIN_DEFINE_SPEC(FSBCombatTestsSpec, "Sandbox.Combat", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBAttributeComponent* AttributeComponent;
	USBTestCombatComponent* CombatComponent;

	FGameplayTag RifleTag;
	FGameplayTag PistolTag;
	FGameplayTag AmmoTag;
END_DEFINE_SPEC(FSBCombatTestsSpec)

void FSBCombatTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		
		// Instancia os componentes
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();
		
		AttributeComponent = NewObject<USBAttributeComponent>(TestCharacter);
		AttributeComponent->RegisterComponent();

		CombatComponent = NewObject<USBTestCombatComponent>(TestCharacter);
		CombatComponent->RegisterComponent();

		// Inicialização
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(AttributeComponent);
		ISBComponentInterface::Execute_OnInitialize(CombatComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(AttributeComponent);
		ISBComponentInterface::Execute_OnReady(CombatComponent);

		// Inicializa tags
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		RifleTag = TagsManager.AddNativeGameplayTag(TEXT("Combat.Action.RifleFire"));
		PistolTag = TagsManager.AddNativeGameplayTag(TEXT("Combat.Action.PistolFire"));
		AmmoTag = FGameplayTag::RequestGameplayTag(TEXT("Attribute.Weapon.Ammo"));
		TagsManager.AddNativeGameplayTag(TEXT("Combat.Group.ActiveWeapon"));

		// Configura atributos de munição
		FSBAttribute AmmoAttr;
		AmmoAttr.BaseValue = 30.f;
		AmmoAttr.CurrentValue = 30.f;
		AmmoAttr.MaxValue = 30.f;
		AmmoAttr.MinValue = 0.f;
		AttributeComponent->RegisterAttribute(AmmoTag, AmmoAttr);

		// Cria definições de arma e carrega a configuração
		USBCombatConfigDataAsset* CombatConfig = NewObject<USBCombatConfigDataAsset>();

		USBWeaponBehaviorDefinition* RifleDef = NewObject<USBWeaponBehaviorDefinition>();
		RifleDef->BehaviorTag = RifleTag;
		RifleDef->StackPriority = 50;
		RifleDef->ExclusivityGroup = FGameplayTag::RequestGameplayTag(TEXT("Combat.Group.ActiveWeapon"));
		RifleDef->AmmoCost = 1.0f;
		RifleDef->Damage = 20.f;
		RifleDef->FireRate = 0.2f;

		USBWeaponBehaviorDefinition* PistolDef = NewObject<USBWeaponBehaviorDefinition>();
		PistolDef->BehaviorTag = PistolTag;
		PistolDef->StackPriority = 30;
		PistolDef->ExclusivityGroup = FGameplayTag::RequestGameplayTag(TEXT("Combat.Group.ActiveWeapon"));
		PistolDef->AmmoCost = 1.0f;
		PistolDef->Damage = 15.f;
		PistolDef->FireRate = 0.4f;

		FSBWeaponConfigEntry RifleEntry;
		RifleEntry.BehaviorClass = USBWeaponBehaviorHitscan::StaticClass();
		RifleEntry.DefinitionAsset = RifleDef;
		CombatConfig->ConfiguredWeapons.Add(RifleEntry);

		FSBWeaponConfigEntry PistolEntry;
		PistolEntry.BehaviorClass = USBWeaponBehaviorHitscan::StaticClass();
		PistolEntry.DefinitionAsset = PistolDef;
		CombatConfig->ConfiguredWeapons.Add(PistolEntry);

		CombatComponent->LoadCombatConfig(CombatConfig);
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

	It("Cenário 1: Predição e Confirmação Jitter-Free do Consumo de Munição", [this]()
	{
		// Configura o Pawn como Cliente Local (Autonomous Proxy)
		TestCharacter->SetRole(ROLE_AutonomousProxy);
		
		APlayerController* LocalPC = TestWorld->SpawnActor<APlayerController>();
		LocalPC->Possess(TestCharacter);

		// 1. Cliente atira com o Rifle (predição) com PredictionId = 100
		bool bRequestSuccess = CombatComponent->RequestWeaponBehavior(RifleTag, 100);
		
		TestTrue("Rifle deve disparar localmente", bRequestSuccess);
		TestTrue("Rifle deve estar na pilha ativa", CombatComponent->HasWeaponBehavior(RifleTag));
		TestTrue("Tag de disparando deve subir no StateComponent", StateComponent->HasTag(FGameplayTag::RequestGameplayTag(TEXT("State.Weapon.Firing"))));
		TestEqual("HUD deve descontar a munição de forma predita", AttributeComponent->GetAttributeValue(AmmoTag), 29.0f);

		// 2. Simula o Servidor processando o disparo autoritativo
		// No servidor, o consumo decrementa o BaseValue e confirma a predição
		AttributeComponent->SetAttributeBaseValue(AmmoTag, 29.0f);
		AttributeComponent->ConfirmPrediction(AmmoTag, 100);

		// 3. Pacotes chegam no cliente (OnReps chamados na mesma tick de rede)
		AttributeComponent->OnRep_ReplicatedAttributes();
		AttributeComponent->OnRep_ConfirmedPredictions();

		// Valida estabilização do valor e remoção da fila de predição
		TestEqual("HUD deve continuar exibindo exatamente 29.f", AttributeComponent->GetAttributeValue(AmmoTag), 29.0f);
		TestEqual("Transações pendentes de atributos devem ser limpas", AttributeComponent->GetPendingPredictions().Num(), 0);
	});

	It("Cenário 2: Rejeição de Disparo por Falta de Munição e Rollback (Anti-Cheat)", [this]()
	{
		// Configura o Pawn como Cliente Local
		TestCharacter->SetRole(ROLE_AutonomousProxy);
		
		APlayerController* LocalPC = TestWorld->SpawnActor<APlayerController>();
		LocalPC->Possess(TestCharacter);

		// Zera munição do cliente local e do servidor
		AttributeComponent->SetAttributeBaseValue(AmmoTag, 0.0f);

		// 1. Cliente tenta atirar sem munição de forma fraudulenta (ignorando CanEnter local)
		// Vamos simular a predição forçando a entrada no cliente
		bool bRequest = CombatComponent->RequestWeaponBehavior(RifleTag, 101);
		TestFalse("Não deve permitir disparo predito se a validação CanEnter do cliente falhar", bRequest);

		// 2. Servidor recebe ServerRequestFire com munição zero
		// Simula rejeição chamando diretamente a verificação do servidor que rejeitará
		CombatComponent->ServerRequestFire_Implementation(RifleTag, 101);

		// Valida que o servidor disparou o rollback de volta para o cliente
		TestEqual("Servidor deve ter rejeitado e disparado Rollback no cliente", CombatComponent->LastRollbackTag, RifleTag);
		TestEqual("ID do Rollback deve coincidir", CombatComponent->LastRollbackPredictionId, 101);

		// Valida que o cliente reverteu a munição para 0 e a pilha está vazia
		TestFalse("Rifle não deve estar na pilha ativa", CombatComponent->HasWeaponBehavior(RifleTag));
		TestEqual("Munição deve permanecer 0", AttributeComponent->GetAttributeValue(AmmoTag), 0.0f);
	});

	It("Cenário 3: Swap de Armas via ExclusivityGroup", [this]()
	{
		// Configura o Pawn como Servidor Autoritativo
		TestCharacter->SetRole(ROLE_Authority);

		// 1. Equipar e disparar com o Rifle
		bool bRifleFire = CombatComponent->RequestWeaponBehavior(RifleTag);
		TestTrue("Rifle disparado com sucesso", bRifleFire);
		TestTrue("Rifle ativo na pilha", CombatComponent->HasWeaponBehavior(RifleTag));

		// 2. Requisitar disparo com a Pistola (mesmo ExclusivityGroup)
		bool bPistolFire = CombatComponent->RequestWeaponBehavior(PistolTag);
		TestTrue("Pistola disparada com sucesso", bPistolFire);
		
		// Verificação de Ejeção por ExclusivityGroup
		TestFalse("Rifle deve ter sido ejetado da pilha", CombatComponent->HasWeaponBehavior(RifleTag));
		TestTrue("Pistola deve assumir o controle ativo", CombatComponent->HasWeaponBehavior(PistolTag));
	});
}
