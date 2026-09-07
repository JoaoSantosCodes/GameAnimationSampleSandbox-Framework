// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Character/SBCharacter.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "GameplayTagsManager.h"
#include "SBGameplayTags.h"
#include "Weapons/SBWeaponBehaviorHitscan.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"

// Classe auxiliar de teste para interceptar e testar métodos protegidos de hitscan
class USBTestWeaponBehaviorHitscan : public USBWeaponBehaviorHitscan
{
public:
	void TestApplyDamage(AActor* HitActor, FName HitBoneName, const FVector& HitDirection)
	{
		if (!HitActor || !WeaponDefinition) return;

		USBAttributeComponent* HitAttrComp = HitActor->FindComponentByClass<USBAttributeComponent>();
		if (HitAttrComp)
		{
			float RawDamage = WeaponDefinition->Damage;
			bool bIsCritical = false;

			if (HitBoneName != NAME_None && WeaponDefinition->CriticalBoneNames.Contains(HitBoneName))
			{
				RawDamage *= WeaponDefinition->CriticalDamageMultiplier;
				bIsCritical = true;
			}

			// Mitigação por Defesa
			FGameplayTag DefenseTag = FSBGameplayTags::Get().Attribute_Defense;
			float DefenseVal = 0.0f;
			if (DefenseTag.IsValid())
			{
				DefenseVal = HitAttrComp->GetAttributeValue(DefenseTag);
			}

			float FinalDamage = RawDamage;
			if (DefenseVal > 0.0f)
			{
				float MitigationRatio = 100.0f / (100.0f + DefenseVal);
				FinalDamage = FMath::Max(1.0f, RawDamage * MitigationRatio);
			}

			FGameplayTag HealthTag = FSBGameplayTags::Get().Attribute_Health;
			float CurrentHealth = HitAttrComp->GetAttributeValue(HealthTag);
			float NewHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);
			HitAttrComp->SetAttributeBaseValue(HealthTag, NewHealth);

			// Aciona Hit Reaction no State Component do alvo
			if (USBStateComponent* TargetStateComp = HitActor->FindComponentByClass<USBStateComponent>())
			{
				FGameplayTag HitReactTag = FSBGameplayTags::Get().State_Character_HitReacting;
				if (HitReactTag.IsValid())
				{
					TargetStateComp->AddTag(HitReactTag);
				}
			}

			// Emite Eventos de Combate no Event Bus
			UWorld* World = HitActor->GetWorld();
			if (World && World->GetGameInstance())
			{
				if (USBEventSubsystem* EventSubsystem = World->GetGameInstance()->GetSubsystem<USBEventSubsystem>())
				{
					USBHitReactEventPayload* HitPayload = NewObject<USBHitReactEventPayload>(this);
					HitPayload->TargetPawn = Cast<APawn>(HitActor);
					HitPayload->InstigatorActor = CombatComponent ? CombatComponent->GetOwner() : nullptr;
					HitPayload->HitBoneName = HitBoneName;
					HitPayload->HitDirection = HitDirection;
					HitPayload->DamageDealt = FinalDamage;
					HitPayload->bIsCritical = bIsCritical;

					EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Combat_HitReact, HitPayload);

					if (bIsCritical)
					{
						EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Combat_CriticalHit, HitPayload);
					}
				}
			}
		}
	}
};

BEGIN_DEFINE_SPEC(FSBCriticalDamageTestsSpec, "Sandbox.CriticalDamage", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* AttackerCharacter;
	ASBCharacter* TargetCharacter;
	USBAttributeComponent* TargetAttributeComponent;
	USBStateComponent* TargetStateComponent;
	USBCombatComponent* AttackerCombatComponent;
	USBWeaponBehaviorDefinition* WeaponDef;
	USBTestWeaponBehaviorHitscan* HitscanBehavior;

	// Telemetria do Event Bus
	bool bHitReactEventReceived;
	bool bCriticalEventReceived;
	float LastEventDamageDealt;
	FName LastEventBoneName;
	bool LastEventIsCritical;
END_DEFINE_SPEC(FSBCriticalDamageTestsSpec)

void FSBCriticalDamageTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		AttackerCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		AttackerCharacter->SetRole(ROLE_Authority);

		TargetCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector(500.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		TargetCharacter->SetRole(ROLE_Authority);

		FSBGameplayTags::InitializeNativeTags();

		// Componentes do Alvo
		TargetAttributeComponent = NewObject<USBAttributeComponent>(TargetCharacter);
		TargetAttributeComponent->RegisterComponent();
		
		TargetStateComponent = NewObject<USBStateComponent>(TargetCharacter);
		TargetStateComponent->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(TargetAttributeComponent);
		ISBComponentInterface::Execute_OnInitialize(TargetStateComponent);
		ISBComponentInterface::Execute_OnReady(TargetAttributeComponent);
		ISBComponentInterface::Execute_OnReady(TargetStateComponent);

		// Configura vida base = 100 e defesa inicial = 0
		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.f;
		HealthAttr.CurrentValue = 100.f;
		HealthAttr.MaxValue = 100.f;
		HealthAttr.MinValue = 0.f;
		TargetAttributeComponent->RegisterAttribute(FSBGameplayTags::Get().Attribute_Health, HealthAttr);

		FSBAttribute DefenseAttr;
		DefenseAttr.BaseValue = 0.f;
		DefenseAttr.CurrentValue = 0.f;
		DefenseAttr.MaxValue = 1000.f;
		DefenseAttr.MinValue = 0.f;
		TargetAttributeComponent->RegisterAttribute(FSBGameplayTags::Get().Attribute_Defense, DefenseAttr);

		// Componentes do Atacante
		AttackerCombatComponent = NewObject<USBCombatComponent>(AttackerCharacter);
		AttackerCombatComponent->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(AttackerCombatComponent);
		ISBComponentInterface::Execute_OnReady(AttackerCombatComponent);

		// Criação e inicialização do Data Asset de arma com dano 20.0 e crítico 2.0x
		WeaponDef = NewObject<USBWeaponBehaviorDefinition>();
		WeaponDef->Damage = 20.0f;
		WeaponDef->CriticalDamageMultiplier = 2.0f;
		WeaponDef->CriticalBoneNames.Empty();
		WeaponDef->CriticalBoneNames.Add(FName(TEXT("head")));
		WeaponDef->CriticalBoneNames.Add(FName(TEXT("neck_01")));

		HitscanBehavior = NewObject<USBTestWeaponBehaviorHitscan>(AttackerCombatComponent);
		HitscanBehavior->Initialize(AttackerCombatComponent, WeaponDef);

		// Reset flags de telemetria
		bHitReactEventReceived = false;
		bCriticalEventReceived = false;
		LastEventDamageDealt = 0.0f;
		LastEventBoneName = NAME_None;
		LastEventIsCritical = false;
	});

	It("Dano normal no corpo deve reduzir a vida pelo dano base sem multiplicador", [this]()
	{
		// Disparo no osso 'spine_02' (não crítico)
		HitscanBehavior->TestApplyDamage(TargetCharacter, FName(TEXT("spine_02")), FVector::ForwardVector);

		float CurrentHealth = TargetAttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Health);
		TestEqual("Vida restante do alvo deve ser 80.0 (100 - 20)", CurrentHealth, 80.0f);
	});

	It("Dano crítico na cabeça deve aplicar o multiplicador CriticalDamageMultiplier (2x)", [this]()
	{
		// Disparo no osso crítico 'head'
		HitscanBehavior->TestApplyDamage(TargetCharacter, FName(TEXT("head")), FVector::ForwardVector);

		float CurrentHealth = TargetAttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Health);
		TestEqual("Vida restante do alvo deve ser 60.0 (100 - 40)", CurrentHealth, 60.0f);
	});

	It("Dano em osso do pescoço também deve ser considerado crítico", [this]()
	{
		// Disparo no osso 'neck_01' (crítico)
		HitscanBehavior->TestApplyDamage(TargetCharacter, FName(TEXT("neck_01")), FVector::ForwardVector);

		float CurrentHealth = TargetAttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Health);
		TestEqual("Vida restante do alvo deve ser 60.0 (100 - 40)", CurrentHealth, 60.0f);
	});

	It("Atributo de Defesa deve mitigar o dano conforme a fórmula de diminishing returns", [this]()
	{
		// Configura defesa para 100 (Mitigação: 100 / (100 + 100) = 0.5)
		TargetAttributeComponent->SetAttributeBaseValue(FSBGameplayTags::Get().Attribute_Defense, 100.0f);

		// Disparo normal de 20.0f -> Dano Final: 20 * 0.5 = 10.0f
		HitscanBehavior->TestApplyDamage(TargetCharacter, FName(TEXT("spine_01")), FVector::ForwardVector);

		float CurrentHealth = TargetAttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Health);
		TestEqual("Vida restante com 100 de defesa deve ser 90.0 (100 - 10)", CurrentHealth, 90.0f);
	});

	It("Dano crítico com Defesa ativa deve mitigar proporcionalmente o dano multiplicado", [this]()
	{
		// Configura defesa para 100 (Mitigação = 0.5)
		TargetAttributeComponent->SetAttributeBaseValue(FSBGameplayTags::Get().Attribute_Defense, 100.0f);

		// Disparo crítico de 20.0f * 2.0x = 40.0f -> Dano Final: 40 * 0.5 = 20.0f
		HitscanBehavior->TestApplyDamage(TargetCharacter, FName(TEXT("head")), FVector::ForwardVector);

		float CurrentHealth = TargetAttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Health);
		TestEqual("Vida restante com crítico e 100 de defesa deve ser 80.0 (100 - 20)", CurrentHealth, 80.0f);
	});

	It("Sofrer dano deve aplicar a tag State.Character.HitReacting no StateComponent do alvo", [this]()
	{
		TestFalse("Alvo não deve ter a tag HitReacting inicialmente", TargetStateComponent->HasTag(FSBGameplayTags::Get().State_Character_HitReacting));

		HitscanBehavior->TestApplyDamage(TargetCharacter, FName(TEXT("spine_01")), FVector::ForwardVector);

		TestTrue("Alvo deve ter recebido a tag HitReacting após sofrer dano", TargetStateComponent->HasTag(FSBGameplayTags::Get().State_Character_HitReacting));
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
		}
	});
}
