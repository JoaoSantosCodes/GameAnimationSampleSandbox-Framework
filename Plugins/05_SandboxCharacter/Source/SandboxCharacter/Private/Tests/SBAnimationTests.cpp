#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SBAnimLayerManagerComponent.h"
#include "Components/SBStateComponent.h"
#include "Animation/ISBAnimLayerInterface.h"
#include "Animation/DataAssets/SBAnimLayerConfigDataAsset.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

// Mocks de AnimInstance para simular Linked Layers are now defined in SBAnimLayerManagerComponent.h

BEGIN_DEFINE_SPEC(FSBAnimationTestsSpec, "Sandbox.Character.Animation", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ACharacter* TestCharacter;
	USBStateComponent* StateComponent;
	USBTestAnimLayerManagerComponent* AnimLayerManager;
	USBAnimLayerConfigDataAsset* AnimConfigDataAsset;

	FGameplayTag CrouchTag;
	FGameplayTag AimTag;
END_DEFINE_SPEC(FSBAnimationTestsSpec)

void FSBAnimationTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ACharacter>(ACharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		
		// Inicializa o skeletal mesh do character com uma AnimInstance básica
		USkeletalMeshComponent* Mesh = TestCharacter->GetMesh();
		if (Mesh)
		{
			Mesh->SetAnimInstanceClass(USBMockAnimInstanceBase::StaticClass());
		}

		// Instancia e registra componentes
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();

		AnimLayerManager = NewObject<USBTestAnimLayerManagerComponent>(TestCharacter);
		
		UAnimInstance* MockAnim = NewObject<UAnimInstance>(TestCharacter->GetMesh());
		AnimLayerManager->MockAnimInstance = MockAnim;

		AnimLayerManager->RegisterComponent();

		// Inicialização
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(AnimLayerManager);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(AnimLayerManager);

		// Cria o Data Asset de teste em memória
		AnimConfigDataAsset = NewObject<USBAnimLayerConfigDataAsset>();
		
		CrouchTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Crouching"));
		AimTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Aiming"));

		// Popula o Data Asset com prioridades
		// Crouch = Prioridade 20
		FSBAnimLayerMappingEntry EntryCrouch;
		EntryCrouch.StateTag = CrouchTag;
		EntryCrouch.AnimLayerClass = USBMockCrouchLayer::StaticClass();
		EntryCrouch.Priority = 20;
		AnimConfigDataAsset->LayerMappings.Add(EntryCrouch);

		// Aim = Prioridade 40
		FSBAnimLayerMappingEntry EntryAim;
		EntryAim.StateTag = AimTag;
		EntryAim.AnimLayerClass = USBMockAimLayer::StaticClass();
		EntryAim.Priority = 40;
		AnimConfigDataAsset->LayerMappings.Add(EntryAim);

		AnimLayerManager->SetConfigAsset(AnimConfigDataAsset);
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

	It("Cenário 1: Re-vinculação sequencial baseada em prioridade (StableSort)", [this]()
	{
		// 1. Ativa as duas tags no StateComponent
		StateComponent->AddTag(CrouchTag);
		StateComponent->AddTag(AimTag);

		// 2. Dispara a reconstrução direta para avaliar a ordem de vinculação
		AnimLayerManager->TriggerDirectRebuild();

		const TArray<TSubclassOf<UAnimInstance>>& LinkedClasses = AnimLayerManager->GetCurrentLinkedClasses();

		// Verificação de que ambas foram incluídas e que Aim (prioridade 40) está no final da lista
		// (Sendo vinculada por último, ela naturalmente ganha o topo da pilha de overrides da Unreal)
		TestEqual("Deve haver 2 layers vinculadas na lista ordenada", LinkedClasses.Num(), 2);
		if (LinkedClasses.Num() == 2)
		{
			TestEqual("Primeira vinculada (menor prioridade) deve ser Crouch", LinkedClasses[0], (TSubclassOf<UAnimInstance>)USBMockCrouchLayer::StaticClass());
			TestEqual("Segunda vinculada (maior prioridade) deve ser Aim", LinkedClasses[1], (TSubclassOf<UAnimInstance>)USBMockAimLayer::StaticClass());
		}
	});

	It("Cenário 2: Coalescimento por Dirty-Flag", [this]()
	{
		// 1. Simula eventos rápidos de tag no mesmo tick
		StateComponent->AddTag(CrouchTag);
		StateComponent->RemoveTag(CrouchTag);
		StateComponent->AddTag(AimTag);

		// 2. Valida se a flag bRebuildPending foi ativada
		TestTrue("Rebuild deve estar pendente após alterações de tags", AnimLayerManager->GetRebuildPending());

		// 3. Roda o Tick do componente e verifica que bRebuildPending foi resetada após processar uma única vez
		AnimLayerManager->TickComponent(0.016f, LEVELTICK_All, nullptr);
		TestFalse("Rebuild não deve mais estar pendente após a execução do tick", AnimLayerManager->GetRebuildPending());
	});
}
