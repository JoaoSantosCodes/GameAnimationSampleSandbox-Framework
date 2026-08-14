#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Weapons/SBWeaponBehavior.h"
#include "Weapons/SBWeaponBehaviorHitscan.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "GameplayTagsManager.h"

#include "Engine/StaticMeshActor.h"

BEGIN_DEFINE_SPEC(FSBWeaponVisualTestsSpec, "Sandbox.Combat.Visuals", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBAttributeComponent* AttributeComponent;
	USBCombatComponent* CombatComponent;

	FGameplayTag RifleTag;
	FGameplayTag WeaponStateTag;
	FGameplayTag AmmoTag;
END_DEFINE_SPEC(FSBWeaponVisualTestsSpec)

void FSBWeaponVisualTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		// Inicializa Tags
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		RifleTag = TagsManager.AddNativeGameplayTag(TEXT("Combat.Action.RifleFire"));
		WeaponStateTag = TagsManager.AddNativeGameplayTag(TEXT("State.Weapon.Firing"));
		AmmoTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Weapon.Ammo"));

		// Instancia componentes
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();
		
		AttributeComponent = NewObject<USBAttributeComponent>(TestCharacter);
		AttributeComponent->RegisterComponent();

		CombatComponent = NewObject<USBCombatComponent>(TestCharacter);
		CombatComponent->RegisterComponent();

		// Inicialização
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(AttributeComponent);
		ISBComponentInterface::Execute_OnInitialize(CombatComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(AttributeComponent);
		ISBComponentInterface::Execute_OnReady(CombatComponent);

		// Configura atributos de munição
		FSBAttribute AmmoAttr;
		AmmoAttr.BaseValue = 30.f;
		AmmoAttr.CurrentValue = 30.f;
		AmmoAttr.MaxValue = 30.f;
		AmmoAttr.MinValue = 0.f;
		AttributeComponent->RegisterAttribute(AmmoTag, AmmoAttr);
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
			TestWorld->DestroyWorld(true);
			TestWorld = nullptr;
		}
	});

	It("Should spawn visual weapon actor, holster it initially, draw on firing, holster on stop, and destroy on cleanup", [this]()
	{
		// 1. Configura Definições de Combate
		USBCombatConfigDataAsset* CombatConfig = NewObject<USBCombatConfigDataAsset>();

		USBWeaponBehaviorDefinition* RifleDef = NewObject<USBWeaponBehaviorDefinition>();
		RifleDef->BehaviorTag = RifleTag;
		RifleDef->StackPriority = 50;
		RifleDef->AmmoCost = 1.0f;
		RifleDef->Damage = 20.f;
		RifleDef->FireRate = 0.2f;

		// Configurações Estéticas
		RifleDef->WeaponActorClass = AStaticMeshActor::StaticClass();
		RifleDef->ActiveSocketName = FName(TEXT("hand_rSocket"));
		RifleDef->HolsterSocketName = FName(TEXT("spine_03Socket"));

		FSBWeaponConfigEntry RifleEntry;
		RifleEntry.BehaviorClass = USBWeaponBehaviorHitscan::StaticClass();
		RifleEntry.DefinitionAsset = RifleDef;
		CombatConfig->ConfiguredWeapons.Add(RifleEntry);

		// 2. Carrega a configuração de combate e verifica o spawn do actor visual
		CombatComponent->LoadCombatConfig(CombatConfig);

		AActor* SpawnedWeapon = CombatComponent->GetSpawnedWeaponActor(RifleTag);
		TestNotNull("Actor da arma visual deve ter sido spawnado no servidor", SpawnedWeapon);

		// Verifica anexação inicial (Coldre)
		USceneComponent* AttachParent = SpawnedWeapon->GetRootComponent() ? SpawnedWeapon->GetRootComponent()->GetAttachParent() : nullptr;
		TestNotNull("Arma deve estar anexada a um componente", AttachParent);
		TestEqual("Arma deve estar anexada no coldre inicialmente", SpawnedWeapon->GetAttachParentSocketName(), FName(TEXT("spine_03Socket")));

		// 3. Inicia o disparo/comportamento da arma
		bool bFired = CombatComponent->RequestWeaponBehavior(RifleTag);
		TestTrue("Disparo deve ser iniciado com sucesso", bFired);

		// Verifica anexação ativa (Mão)
		TestEqual("Arma deve ser sacada para a mão no disparo", SpawnedWeapon->GetAttachParentSocketName(), FName(TEXT("hand_rSocket")));

		// 4. Finaliza o disparo
		CombatComponent->StopWeaponBehavior(RifleTag);

		// Verifica anexação de volta ao coldre
		TestEqual("Arma deve retornar para o coldre ao parar disparo", SpawnedWeapon->GetAttachParentSocketName(), FName(TEXT("spine_03Socket")));

		// 5. Destruição por Shutdown do componente
		TWeakObjectPtr<AActor> WeakWeapon = SpawnedWeapon;
		ISBComponentInterface::Execute_OnShutdown(CombatComponent);

		TestFalse("Actor visual da arma deve ter sido destruído no shutdown do componente", WeakWeapon.IsValid());
	});
}
