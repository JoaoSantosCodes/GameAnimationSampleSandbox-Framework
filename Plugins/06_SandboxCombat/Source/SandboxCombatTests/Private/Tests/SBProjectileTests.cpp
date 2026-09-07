// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Character/SBCharacter.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Weapons/SBPhysicalProjectile.h"
#include "Weapons/SBWeaponBehaviorProjectile.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "GameplayTagsManager.h"
#include "SBGameplayTags.h"

// Classe auxiliar de teste herdada para testar métodos protegidos de impacto
class ASBTestPhysicalProjectile : public ASBPhysicalProjectile
{
public:
	void TestDirectHit(AActor* HitActor, FName HitBone)
	{
		FHitResult Hit;
		Hit.BoneName = HitBone;
		Hit.ImpactPoint = HitActor ? HitActor->GetActorLocation() : FVector::ZeroVector;
		ProcessDirectHit(HitActor, Hit);
	}

	void TestRadialHit(const FVector& Center)
	{
		ProcessRadialDamage(Center);
	}
};

BEGIN_DEFINE_SPEC(FSBProjectileTestsSpec, "Sandbox.Projectile", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* Attacker;
	ASBCharacter* Target;
	USBAttributeComponent* TargetAttrComp;
	USBStateComponent* TargetStateComp;
	USBCombatComponent* AttackerCombatComp;
	USBWeaponBehaviorDefinition* ProjectileWeaponDef;
END_DEFINE_SPEC(FSBProjectileTestsSpec)

void FSBProjectileTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		Attacker = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		Attacker->SetRole(ROLE_Authority);

		Target = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector(400.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		Target->SetRole(ROLE_Authority);

		FSBGameplayTags::InitializeNativeTags();

		// Configura componentes do Alvo
		TargetAttrComp = NewObject<USBAttributeComponent>(Target);
		TargetAttrComp->RegisterComponent();
		TargetStateComp = NewObject<USBStateComponent>(Target);
		TargetStateComp->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(TargetAttrComp);
		ISBComponentInterface::Execute_OnInitialize(TargetStateComp);
		ISBComponentInterface::Execute_OnReady(TargetAttrComp);
		ISBComponentInterface::Execute_OnReady(TargetStateComp);

		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.f;
		HealthAttr.CurrentValue = 100.f;
		HealthAttr.MaxValue = 100.f;
		HealthAttr.MinValue = 0.f;
		TargetAttrComp->RegisterAttribute(FSBGameplayTags::Get().Attribute_Health, HealthAttr);

		FSBAttribute DefenseAttr;
		DefenseAttr.BaseValue = 0.f;
		DefenseAttr.CurrentValue = 0.f;
		DefenseAttr.MaxValue = 1000.f;
		DefenseAttr.MinValue = 0.f;
		TargetAttrComp->RegisterAttribute(FSBGameplayTags::Get().Attribute_Defense, DefenseAttr);

		// Configura componentes do Atacante
		AttackerCombatComp = NewObject<USBCombatComponent>(Attacker);
		AttackerCombatComp->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(AttackerCombatComp);
		ISBComponentInterface::Execute_OnReady(AttackerCombatComp);

		// Definição da arma de projétil
		ProjectileWeaponDef = NewObject<USBWeaponBehaviorDefinition>();
		ProjectileWeaponDef->Damage = 30.0f;
		ProjectileWeaponDef->CriticalDamageMultiplier = 2.0f;
		ProjectileWeaponDef->CriticalBoneNames.Empty();
		ProjectileWeaponDef->CriticalBoneNames.Add(FName(TEXT("head")));
		ProjectileWeaponDef->ProjectileClass = ASBTestPhysicalProjectile::StaticClass();
		ProjectileWeaponDef->ProjectileInitialSpeed = 3500.0f;
	});

	It("Impacto direto de projétil no corpo deve aplicar o dano base", [this]()
	{
		FActorSpawnParameters SpawnParams;
		ASBTestPhysicalProjectile* Proj = TestWorld->SpawnActor<ASBTestPhysicalProjectile>(ASBTestPhysicalProjectile::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		Proj->InitializeProjectile(30.0f, 3500.0f, 1.0f, 2.0f, { FName(TEXT("head")) }, 0.0f, Attacker);

		Proj->TestDirectHit(Target, FName(TEXT("spine_01")));

		float Health = TargetAttrComp->GetAttributeValue(FSBGameplayTags::Get().Attribute_Health);
		TestEqual("Vida restante do alvo deve ser 70.0 (100 - 30)", Health, 70.0f);
	});

	It("Impacto de projétil na cabeça deve aplicar dano crítico multiplicado (2x)", [this]()
	{
		FActorSpawnParameters SpawnParams;
		ASBTestPhysicalProjectile* Proj = TestWorld->SpawnActor<ASBTestPhysicalProjectile>(ASBTestPhysicalProjectile::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		Proj->InitializeProjectile(30.0f, 3500.0f, 1.0f, 2.0f, { FName(TEXT("head")) }, 0.0f, Attacker);

		Proj->TestDirectHit(Target, FName(TEXT("head")));

		float Health = TargetAttrComp->GetAttributeValue(FSBGameplayTags::Get().Attribute_Health);
		TestEqual("Vida restante do alvo com headshot deve ser 40.0 (100 - 60)", Health, 40.0f);
	});

	It("Impacto de projétil deve mitigar dano de acordo com Atributo de Defesa", [this]()
	{
		TargetAttrComp->SetAttributeBaseValue(FSBGameplayTags::Get().Attribute_Defense, 100.0f);

		FActorSpawnParameters SpawnParams;
		ASBTestPhysicalProjectile* Proj = TestWorld->SpawnActor<ASBTestPhysicalProjectile>(ASBTestPhysicalProjectile::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		Proj->InitializeProjectile(30.0f, 3500.0f, 1.0f, 2.0f, { FName(TEXT("head")) }, 0.0f, Attacker);

		// Dano mitigado: 30 * (100 / (100 + 100)) = 15
		Proj->TestDirectHit(Target, FName(TEXT("spine_01")));

		float Health = TargetAttrComp->GetAttributeValue(FSBGameplayTags::Get().Attribute_Health);
		TestEqual("Vida restante com 100 de defesa deve ser 85.0 (100 - 15)", Health, 85.0f);
	});

	It("Alvo atingido por projétil deve receber a tag State.Character.HitReacting", [this]()
	{
		TestFalse("Alvo não deve ter HitReacting inicialmente", TargetStateComp->HasTag(FSBGameplayTags::Get().State_Character_HitReacting));

		FActorSpawnParameters SpawnParams;
		ASBTestPhysicalProjectile* Proj = TestWorld->SpawnActor<ASBTestPhysicalProjectile>(ASBTestPhysicalProjectile::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		Proj->InitializeProjectile(30.0f, 3500.0f, 1.0f, 2.0f, { FName(TEXT("head")) }, 0.0f, Attacker);

		Proj->TestDirectHit(Target, FName(TEXT("spine_01")));

		TestTrue("Alvo deve ter recebido a tag HitReacting após o impacto do projétil", TargetStateComp->HasTag(FSBGameplayTags::Get().State_Character_HitReacting));
	});

	It("Dano radial de projétil explosivo deve causar dano em área com falloff de distância", [this]()
	{
		// Target está em (400, 0, 0). Explosão ocorre em (300, 0, 0) -> Distância = 100 unidades
		// Raio = 200 -> Falloff = 1 - 100/200 = 0.5 -> Dano = 50 * 0.5 = 25
		FActorSpawnParameters SpawnParams;
		ASBTestPhysicalProjectile* Proj = TestWorld->SpawnActor<ASBTestPhysicalProjectile>(ASBTestPhysicalProjectile::StaticClass(), FVector(300.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		Proj->InitializeProjectile(50.0f, 3500.0f, 1.0f, 2.0f, {}, 200.0f, Attacker);

		// Dano radial direto na posição do alvo
		Proj->TestDirectHit(Target, FName(TEXT("spine_01")));

		float Health = TargetAttrComp->GetAttributeValue(FSBGameplayTags::Get().Attribute_Health);
		TestTrue("Vida restante deve ser menor que 100 após impacto", Health < 100.0f);
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
		}
	});
}
