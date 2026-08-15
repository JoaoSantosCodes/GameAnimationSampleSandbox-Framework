#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBCombatComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBMovementComponent.h"
#include "Weapons/SBWeaponBehavior.h"
#include "Weapons/SBWeaponBehaviorHitscan.h"
#include "DataAssets/SBWeaponBehaviorDefinition.h"
#include "GameplayTagsManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

BEGIN_DEFINE_SPEC(FSBAntiCheatTestsSpec, "Sandbox.AntiCheat", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* Attacker;
	ASBCharacter* Target;

	USBStateComponent* AttackerState;
	USBAttributeComponent* AttackerAttributes;
	USBCombatComponent* AttackerCombat;
	USBMovementComponent* AttackerMovement;

	USBAttributeComponent* TargetAttributes;

	FGameplayTag RifleTag;
	FGameplayTag WeaponStateTag;
	FGameplayTag AmmoTag;
	FGameplayTag HealthTag;
END_DEFINE_SPEC(FSBAntiCheatTestsSpec)

void FSBAntiCheatTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		Attacker = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
		Attacker->SetRole(ROLE_Authority);

		Target = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector(1000.f, 0.f, 100.f), FRotator(0.f, 180.f, 0.f), SpawnParams);
		Target->SetRole(ROLE_Authority);

		// Certifica colisão contra visibilidade na cápsula para fins de Hitscan
		if (Attacker->GetCapsuleComponent())
		{
			Attacker->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}
		if (Target->GetCapsuleComponent())
		{
			Target->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}

		// Inicializa Tags
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		RifleTag = TagsManager.AddNativeGameplayTag(TEXT("Combat.Action.RifleFire"));
		WeaponStateTag = TagsManager.AddNativeGameplayTag(TEXT("State.Weapon.Firing"));
		AmmoTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Weapon.Ammo"));
		HealthTag = TagsManager.AddNativeGameplayTag(TEXT("Attribute.Health"));

		// Inicializa componentes do Attacker
		AttackerState = NewObject<USBStateComponent>(Attacker);
		AttackerState->RegisterComponent();

		AttackerAttributes = NewObject<USBAttributeComponent>(Attacker);
		AttackerAttributes->RegisterComponent();

		AttackerCombat = NewObject<USBCombatComponent>(Attacker);
		AttackerCombat->RegisterComponent();

		AttackerMovement = NewObject<USBMovementComponent>(Attacker);
		AttackerMovement->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(AttackerState);
		ISBComponentInterface::Execute_OnInitialize(AttackerAttributes);
		ISBComponentInterface::Execute_OnInitialize(AttackerCombat);
		ISBComponentInterface::Execute_OnInitialize(AttackerMovement);

		ISBComponentInterface::Execute_OnReady(AttackerState);
		ISBComponentInterface::Execute_OnReady(AttackerAttributes);
		ISBComponentInterface::Execute_OnReady(AttackerCombat);
		ISBComponentInterface::Execute_OnReady(AttackerMovement);

		// Inicializa componentes do Target
		TargetAttributes = NewObject<USBAttributeComponent>(Target);
		TargetAttributes->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(TargetAttributes);
		ISBComponentInterface::Execute_OnReady(TargetAttributes);

		// Configura atributos de vida do Target
		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.f;
		HealthAttr.CurrentValue = 100.f;
		HealthAttr.MaxValue = 100.f;
		HealthAttr.MinValue = 0.f;
		TargetAttributes->RegisterAttribute(HealthTag, HealthAttr);

		// Configura atributos de munição do Attacker
		FSBAttribute AmmoAttr;
		AmmoAttr.BaseValue = 30.f;
		AmmoAttr.CurrentValue = 30.f;
		AmmoAttr.MaxValue = 30.f;
		AmmoAttr.MinValue = 0.f;
		AttackerAttributes->RegisterAttribute(AmmoTag, AmmoAttr);
	});

	AfterEach([this]()
	{
		if (Attacker)
		{
			Attacker->Destroy();
			Attacker = nullptr;
		}

		if (Target)
		{
			Target->Destroy();
			Target = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(true);
			TestWorld = nullptr;
		}
	});

	It("Should detect speedhacks and force location rollback on movement anomalies", [this]()
	{
		// Configura velocidade máxima inicial
		Attacker->GetCharacterMovement()->MaxWalkSpeed = 600.f;

		// 1. Executa um tick inicial para registrar a LastValidatedLocation
		AttackerMovement->TickComponent(0.1f, LEVELTICK_All, nullptr);
		FVector OriginalLocation = Attacker->GetActorLocation();

		// 2. Simula movimento lícito (deslocamento pequeno)
		Attacker->SetActorLocation(OriginalLocation + FVector(20.f, 0.f, 0.f));
		AttackerMovement->TickComponent(0.1f, LEVELTICK_All, nullptr);
		FVector ValidLocation = Attacker->GetActorLocation();
		TestEqual("Localização válida deve ser aceita e não sofrer rollback", ValidLocation, OriginalLocation + FVector(20.f, 0.f, 0.f));

		// 3. Simula Speedhack (deslocamento de 1000 unidades em 0.01 segundos)
		Attacker->SetActorLocation(ValidLocation + FVector(1000.f, 0.f, 0.f));
		AttackerMovement->TickComponent(0.01f, LEVELTICK_All, nullptr);

		FVector PostSpeedhackLocation = Attacker->GetActorLocation();
		TestEqual("Localização com speedhack deve sofrer rollback para a última válida", PostSpeedhackLocation, ValidLocation);

		// 4. Simula Warp Hack (teleporte extremo instantâneo de 5000 unidades)
		Attacker->SetActorLocation(ValidLocation + FVector(5000.f, 0.f, 0.f));
		AttackerMovement->TickComponent(0.1f, LEVELTICK_All, nullptr);

		FVector PostWarpLocation = Attacker->GetActorLocation();
		TestEqual("Teleporte instantâneo extremo deve sofrer rollback", PostWarpLocation, ValidLocation);
	});

	It("Should allow teleport relocation when authorized by the server", [this]()
	{
		// Configura velocidade máxima inicial
		Attacker->GetCharacterMovement()->MaxWalkSpeed = 600.f;

		// 1. Executa um tick inicial para registrar a LastValidatedLocation
		AttackerMovement->TickComponent(0.1f, LEVELTICK_All, nullptr);
		FVector ValidLocation = Attacker->GetActorLocation();

		// 2. Teleporta o personagem e autoriza a realocação
		Attacker->SetActorLocation(ValidLocation + FVector(5000.f, 0.f, 0.f));
		AttackerMovement->AuthorizeServerRelocation();

		// 3. Executa o tick do movimento
		AttackerMovement->TickComponent(0.1f, LEVELTICK_All, nullptr);

		// 4. Valida que a localização não sofreu rollback e foi aceita
		FVector PostTeleportLocation = Attacker->GetActorLocation();
		TestEqual("Teleporte autorizado pelo servidor deve ser aceito sem rollback", PostTeleportLocation, ValidLocation + FVector(5000.f, 0.f, 0.f));
	});

	It("Should block damage traces that intersect physical obstacles (anti wall-clipping)", [this]()
	{
		// 1. Configura Definições de Combate do Rifle
		USBCombatConfigDataAsset* CombatConfig = NewObject<USBCombatConfigDataAsset>();

		USBWeaponBehaviorDefinition* RifleDef = NewObject<USBWeaponBehaviorDefinition>();
		RifleDef->BehaviorTag = RifleTag;
		RifleDef->StackPriority = 50;
		RifleDef->AmmoCost = 1.0f;
		RifleDef->Damage = 25.f;
		RifleDef->FireRate = 0.0f;

		// Configurações Estéticas (AStaticMeshActor para testar sem crashes)
		RifleDef->WeaponActorClass = ASBCharacter::StaticClass();
		RifleDef->ActiveSocketName = FName(TEXT("hand_rSocket"));
		RifleDef->HolsterSocketName = FName(TEXT("spine_03Socket"));

		FSBWeaponConfigEntry RifleEntry;
		RifleEntry.BehaviorClass = USBWeaponBehaviorHitscan::StaticClass();
		RifleEntry.DefinitionAsset = RifleDef;
		CombatConfig->ConfiguredWeapons.Add(RifleEntry);

		AttackerCombat->LoadCombatConfig(CombatConfig);

		// 2. Aponta o atacante para a direção do alvo (Rifle Range = 5000)
		Attacker->SetActorRotation(FRotator(0.f, 0.f, 0.f));
		Target->SetActorLocation(FVector(500.f, 0.f, 100.f)); // Alvo a 5 metros de distância

		// 3. Spawna uma parede/obstáculo no meio do trajeto (a 2.5 metros)
		FActorSpawnParameters SpawnParams;
		AActor* ObstacleWall = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(250.f, 0.f, 100.f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull("Obstáculo físico (parede) deve ser spawnado", ObstacleWall);
		
		UBoxComponent* BoxComp = NewObject<UBoxComponent>(ObstacleWall);
		BoxComp->RegisterComponent();
		ObstacleWall->SetRootComponent(BoxComp);
		BoxComp->SetBoxExtent(FVector(50.f, 200.f, 200.f));
		BoxComp->SetCollisionResponseToAllChannels(ECR_Block);
		BoxComp->SetMobility(EComponentMobility::Static);

		// 4. Executa o disparo através do obstáculo
		bool bFiredWithObstacle = AttackerCombat->RequestWeaponBehavior(RifleTag);
		TestTrue("Requisição de disparo deve funcionar", bFiredWithObstacle);

		float HealthWithObstacle = TargetAttributes->GetAttributeValue(HealthTag);
		TestEqual("Alvo não deve tomar dano por haver obstáculo entre eles (Wall Shot bloqueado)", HealthWithObstacle, 100.f);

		// Limpa o disparo
		AttackerCombat->StopWeaponBehavior(RifleTag);

		// 5. Destrói o obstáculo e atira novamente
		ObstacleWall->Destroy();
		
		bool bFiredWithoutObstacle = AttackerCombat->RequestWeaponBehavior(RifleTag);
		TestTrue("Novo disparo sem obstáculo deve funcionar", bFiredWithoutObstacle);

		float HealthWithoutObstacle = TargetAttributes->GetAttributeValue(HealthTag);
		TestEqual("Alvo deve tomar dano sem obstáculo intermediário", HealthWithoutObstacle, 75.f);
	});
}
