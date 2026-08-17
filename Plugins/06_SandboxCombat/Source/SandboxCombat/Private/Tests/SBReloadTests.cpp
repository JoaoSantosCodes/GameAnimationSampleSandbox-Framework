#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "GameplayTagsManager.h"
#include "SBGameplayTags.h"
#include "Weapons/SBWeaponBehavior.h"
#include "Weapons/SBWeaponBehaviorHitscan.h"
#include "Weapons/SBWeaponBehaviorReload.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"

BEGIN_DEFINE_SPEC(FSBReloadTestsSpec, "Sandbox.Reloading", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBAttributeComponent* AttributeComponent;
	USBCombatComponent* CombatComponent;

	FGameplayTag AmmoTag;
	FGameplayTag ReloadingTag;
	FGameplayTag FireBehaviorTag;
	FGameplayTag ReloadBehaviorTag;
END_DEFINE_SPEC(FSBReloadTestsSpec)

void FSBReloadTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		// Inicializa Tags
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		AmmoTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Weapon.Ammo"));
		ReloadingTag = TagsManager.AddNativeGameplayTag(TEXT("State.Character.Reloading"));
		FireBehaviorTag = TagsManager.AddNativeGameplayTag(TEXT("Combat.Action.Fire"));
		ReloadBehaviorTag = TagsManager.AddNativeGameplayTag(TEXT("Combat.Action.Reload"));

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

		// Configura atributos de munição iniciais
		FSBAttribute AmmoAttr;
		AmmoAttr.BaseValue = 30.f;
		AmmoAttr.CurrentValue = 30.f;
		AmmoAttr.MaxValue = 30.f;
		AmmoAttr.MinValue = 0.f;
		AmmoAttr.bIsPrivate = true;
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

	It("Should consume ammo when firing and block fire when ammo is 0", [this]()
	{
		// Criamos a definição para simular a arma
		USBWeaponBehaviorDefinition* FireDef = NewObject<USBWeaponBehaviorDefinition>();
		FireDef->BehaviorTag = FireBehaviorTag;
		FireDef->AmmoCost = 5.f;

		// Registra a arma no orquestrador de combate
		USBWeaponBehavior* FireBehavior = NewObject<USBWeaponBehaviorHitscan>(CombatComponent);
		FireBehavior->Initialize(CombatComponent, FireDef);
		CombatComponent->AddAvailableBehavior(FireBehavior);

		// 1. Primeiro disparo (consome 5.f munições)
		bool bFired = CombatComponent->RequestWeaponBehavior(FireBehaviorTag);
		TestTrue("Disparo 1 deve ter sucesso", bFired);
		TestEqual("Munição deve cair para 25", AttributeComponent->GetAttributeValue(AmmoTag), 25.f);

		// 2. Zera a munição manualmente
		AttributeComponent->SetAttributeBaseValue(AmmoTag, 0.f);

		// 3. Segundo disparo (deve falhar por falta de munição)
		bool bFired2 = CombatComponent->RequestWeaponBehavior(FireBehaviorTag);
		TestFalse("Disparo 2 deve falhar sem munição", bFired2);
	});

	It("Should reload ammo to max value and block firing during reload", [this]()
	{
		// Definições de armas e recarga
		USBWeaponBehaviorDefinition* FireDef = NewObject<USBWeaponBehaviorDefinition>();
		FireDef->BehaviorTag = FireBehaviorTag;
		FireDef->AmmoCost = 1.f;

		USBWeaponBehavior* FireBehavior = NewObject<USBWeaponBehaviorHitscan>(CombatComponent);
		FireBehavior->Initialize(CombatComponent, FireDef);
		CombatComponent->AddAvailableBehavior(FireBehavior);

		// Reduz munição para 10
		AttributeComponent->SetAttributeBaseValue(AmmoTag, 10.f);

		// Instancia o comportamento de recarga
		USBWeaponBehaviorReload* ReloadBehavior = NewObject<USBWeaponBehaviorReload>(CombatComponent);
		
		// Criamos uma definição genérica de behavior para recarga
		USBGameplayBehaviorDefinition* ReloadDef = NewObject<USBGameplayBehaviorDefinition>();
		ReloadDef->BehaviorTag = ReloadBehaviorTag;

		ReloadBehavior->Initialize(CombatComponent, ReloadDef);
		CombatComponent->AddAvailableBehavior(ReloadBehavior);

		// 1. Inicia recarga
		bool bReloadStarted = CombatComponent->RequestBehavior(ReloadBehaviorTag);
		TestTrue("Recarga deve iniciar", bReloadStarted);
		TestTrue("Deve possuir a tag de recarregando", StateComponent->HasTag(ReloadingTag));

		// 2. Tenta atirar durante a recarga. Deve ser bloqueado.
		bool bFiredDuringReload = CombatComponent->RequestWeaponBehavior(FireBehaviorTag);
		TestFalse("Disparo deve ser bloqueado durante recarga", bFiredDuringReload);

		// 3. Ticks de tempo passando (duração = 2.0s). Ticked 1s -> não terminou.
		ReloadBehavior->Update(1.0f, FSBBehaviorContext());
		TestTrue("Deve continuar recarregando", StateComponent->HasTag(ReloadingTag));
		TestEqual("Munição deve continuar em 10", AttributeComponent->GetAttributeValue(AmmoTag), 10.f);

		// Ticked mais 1s -> total de 2s. Deve terminar recarga e reestabelecer munição para 30.f (Max).
		ReloadBehavior->Update(1.0f, FSBBehaviorContext());
		TestFalse("Não deve mais possuir a tag de recarregando", StateComponent->HasTag(ReloadingTag));
		TestEqual("Munição deve ser restaurada para 30", AttributeComponent->GetAttributeValue(AmmoTag), 30.f);
	});
}
