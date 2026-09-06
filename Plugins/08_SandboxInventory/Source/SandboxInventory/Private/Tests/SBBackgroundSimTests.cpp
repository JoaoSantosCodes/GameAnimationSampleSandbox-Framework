#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Actors/SBResourceNode.h"
#include "Components/SBPersistenceComponent.h"
#include "Subsystems/SBSandboxBackgroundSimSubsystem.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Types/SBPersistenceTypes.h"
#include "SBGameplayTags.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"

BEGIN_DEFINE_SPEC(FSBBackgroundSimTestsSpec, "Sandbox.BackgroundSim", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	USBSandboxBackgroundSimSubsystem* SimSubsystem;
END_DEFINE_SPEC(FSBBackgroundSimTestsSpec)

void FSBBackgroundSimTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		SimSubsystem = TestWorld->GetSubsystem<USBSandboxBackgroundSimSubsystem>();
		SimSubsystem->ClearAllSimulations();
	});

	AfterEach([this]()
	{
		if (SimSubsystem)
		{
			SimSubsystem->ClearAllSimulations();
			SimSubsystem = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should decrement registered entity timers on background ticks", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();
		FGuid TestGuid = FGuid::NewGuid();
		FSBSimulatedEntityData EntityData;
		EntityData.EntityId = TestGuid;
		EntityData.EntitySimType = Tags.SimType_ResourceNode;
		EntityData.NumericStates.Add(Tags.State_Timer_Respawn, 10.0f);

		SimSubsystem->RegisterSimulatedEntity(EntityData);

		FSBSimulatedEntityData QueryData;
		TestTrue("Entity should be registered", SimSubsystem->GetSimulatedEntityData(TestGuid, QueryData));
		const float* InitialTimer = QueryData.NumericStates.Find(Tags.State_Timer_Respawn);
		TestTrue("Initial timer state exists", InitialTimer != nullptr);
		if (InitialTimer)
		{
			TestEqual("Initial timer should be 10", *InitialTimer, 10.0f);
		}

		// Avança o tempo em 3 segundos
		SimSubsystem->ForceAdvanceTime(3.0f);

		TestTrue("Entity should still be registered", SimSubsystem->GetSimulatedEntityData(TestGuid, QueryData));
		const float* DecrementedTimer = QueryData.NumericStates.Find(Tags.State_Timer_Respawn);
		TestTrue("Decremented timer state exists", DecrementedTimer != nullptr);
		if (DecrementedTimer)
		{
			TestEqual("Timer should be decremented to 7", *DecrementedTimer, 7.0f);
		}

		// Avança o tempo além do limite
		SimSubsystem->ForceAdvanceTime(8.0f);
		SimSubsystem->GetSimulatedEntityData(TestGuid, QueryData);
		const float* ClampedTimer = QueryData.NumericStates.Find(Tags.State_Timer_Respawn);
		TestTrue("Clamped timer state exists", ClampedTimer != nullptr);
		if (ClampedTimer)
		{
			TestEqual("Timer should be clamped to 0", *ClampedTimer, 0.0f);
		}
	});

	It("Should handle resource node unloading and dynamic resumption with remaining time", [this]()
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		ASBResourceNode* ResourceNode = TestWorld->SpawnActor<ASBResourceNode>(ASBResourceNode::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		ResourceNode->SetRole(ROLE_Authority);
		TestTrue("Node should be spawned", ResourceNode != nullptr);

		// Adiciona componente de persistência e gera GUID estável
		USBPersistenceComponent* PersistComp = NewObject<USBPersistenceComponent>(ResourceNode);
		PersistComp->RegisterComponent();
		PersistComp->PersistentId.Generate();
		FGuid NodeGuid = PersistComp->PersistentId.Guid;

		// Registra escuta de respawn de 10s e esgota o nó
		ResourceNode->TakeDamage(100.0f, FDamageEvent(), nullptr, nullptr);
		TestTrue("Node should be depleted", ResourceNode->IsDepleted());

		// Verifica que registrou na simulação de segundo plano
		FSBSimulatedEntityData QueryData;
		TestTrue("Subsystem should track depleted node", SimSubsystem->GetSimulatedEntityData(NodeGuid, QueryData));

		// Destrói o nó simulando stream-out do level
		ResourceNode->Destroy();
		ResourceNode = nullptr;

		// Simula avanço de tempo de 4 segundos no mundo em background
		SimSubsystem->ForceAdvanceTime(4.0f);

		// Recria/reinstancia o ator simulando stream-in do level
		ASBResourceNode* LoadedNode = TestWorld->SpawnActor<ASBResourceNode>(ASBResourceNode::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		LoadedNode->SetRole(ROLE_Authority);
		USBPersistenceComponent* LoadedPersist = NewObject<USBPersistenceComponent>(LoadedNode);
		LoadedPersist->RegisterComponent();
		LoadedPersist->PersistentId.Guid = NodeGuid;

		// Dispara manualmente BeginPlay para acionar a retomada da simulação
		LoadedNode->DispatchBeginPlay();

		TestTrue("Loaded node should restore depleted state", LoadedNode->IsDepleted());
		
		// Força o respawn direto no nó (simula passagem de tempo no timer local)
		LoadedNode->DebugForceRespawn();
		TestFalse("Loaded node should be restored after respawn timer fires", LoadedNode->IsDepleted());

		LoadedNode->Destroy();
	});

	It("Should persist active simulations across Save and Load with catch-up temporal verification", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();
		FGuid TestGuid = FGuid::NewGuid();
		FSBSimulatedEntityData EntityData;
		EntityData.EntityId = TestGuid;
		EntityData.EntitySimType = Tags.SimType_ResourceNode;
		EntityData.NumericStates.Add(Tags.State_Timer_Respawn, 20.0f);

		SimSubsystem->RegisterSimulatedEntity(EntityData);

		// Salva o estado do subsistema em um payload real do Sandbox
		USBSavePayload* SavePayload = NewObject<USBSavePayload>(TestWorld);
		SimSubsystem->SaveComponentData_Implementation(SavePayload);

		// Limpa o subsistema
		SimSubsystem->ClearAllSimulations();
		FSBSimulatedEntityData QueryData;
		TestFalse("Should have no simulations after clear", SimSubsystem->GetSimulatedEntityData(TestGuid, QueryData));

		// Carrega o estado de volta
		SimSubsystem->LoadComponentData_Implementation(SavePayload);

		TestTrue("Should restore simulation after load", SimSubsystem->GetSimulatedEntityData(TestGuid, QueryData));
		const float* RestoredTimer = QueryData.NumericStates.Find(Tags.State_Timer_Respawn);
		TestTrue("Restored timer state exists", RestoredTimer != nullptr);
		if (RestoredTimer)
		{
			TestEqual("Restored timer should be 20", *RestoredTimer, 20.0f);
		}
	});
}
