// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBDynamicTickThrottlingComponent.h"
#include "SBCoreTestTypes.h"
#include "Subsystems/SBDynamicTickManagerSubsystem.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBDynamicTickThrottlingTestsSpec, "Sandbox.Core.DynamicTickThrottling", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBDynamicTickThrottlingComponent* ThrottlingComp;
	USBCoreTestStateComponent* StateComp;
END_DEFINE_SPEC(FSBDynamicTickThrottlingTestsSpec)

void FSBDynamicTickThrottlingTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		TestActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(TestActor, TEXT("Root"));
		TestActor->SetRootComponent(Root);
		Root->RegisterComponent();

		StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
		TestActor->AddOwnedComponent(StateComp);

		ThrottlingComp = NewObject<USBDynamicTickThrottlingComponent>(TestActor, TEXT("ThrottlingComp"));
		TestActor->AddOwnedComponent(ThrottlingComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(ThrottlingComp);
	});

	AfterEach([this]()
	{
		if (TestActor)
		{
			TestActor->Destroy();
			TestActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should initialize at LOD0, execute on every tick, and grant LOD0 tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBTickThrottlingSettings Settings;
		ThrottlingComp->SetupThrottling(Settings);

		TestEqual("Current LOD is LOD0", (int32)ThrottlingComp->GetCurrentLOD(), (int32)ESBTickLODLevel::LOD0_HighPriority);
		TestTrue("Has LOD0 tag", StateComp->HasTag(Tags.State_Throttling_LOD0));

		float ConsDelta = 0.0f;
		bool bTicked = ThrottlingComp->AdvanceTick(0.016f, ConsDelta);

		TestTrue("Tick executes in LOD0", bTicked);
		TestNearlyEqual("Consolidated delta equals frame delta", ConsDelta, 0.016f, 0.001f);
	});

	It("Should transition to LOD1 at medium distance, throttle tick rate to 10Hz, and accumulate delta time", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBTickThrottlingSettings Settings;
		Settings.LOD0_MaxDistance = 1500.0f;
		Settings.LOD1_MaxDistance = 5000.0f;
		Settings.LOD1_Interval = 0.1f;
		ThrottlingComp->SetupThrottling(Settings);

		ThrottlingComp->UpdateDistanceToViewer(3000.0f, true);

		TestEqual("Shifted to LOD1", (int32)ThrottlingComp->GetCurrentLOD(), (int32)ESBTickLODLevel::LOD1_MediumPriority);
		TestTrue("Has LOD1 tag", StateComp->HasTag(Tags.State_Throttling_LOD1));

		float ConsDelta = 0.0f;
		bool bTicked1 = ThrottlingComp->AdvanceTick(0.03f, ConsDelta);

		TestFalse("First subframe does not execute tick", bTicked1);
		TestEqual("Consolidated delta is 0", ConsDelta, 0.0f);

		bool bTicked2 = ThrottlingComp->AdvanceTick(0.08f, ConsDelta);

		TestTrue("Second subframe reaches interval and executes tick", bTicked2);
		TestNearlyEqual("Consolidated delta accumulates total elapsed time", ConsDelta, 0.11f, 0.001f);
	});

	It("Should transition to LOD2 at far distance and LOD3 Background beyond max distance", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBTickThrottlingSettings Settings;
		Settings.LOD0_MaxDistance = 1500.0f;
		Settings.LOD1_MaxDistance = 5000.0f;
		Settings.LOD2_MaxDistance = 15000.0f;
		ThrottlingComp->SetupThrottling(Settings);

		ThrottlingComp->UpdateDistanceToViewer(10000.0f, true);

		TestEqual("Shifted to LOD2", (int32)ThrottlingComp->GetCurrentLOD(), (int32)ESBTickLODLevel::LOD2_LowPriority);
		TestTrue("Has LOD2 tag", StateComp->HasTag(Tags.State_Throttling_LOD2));

		ThrottlingComp->UpdateDistanceToViewer(25000.0f, false);

		TestEqual("Shifted to LOD3 Background", (int32)ThrottlingComp->GetCurrentLOD(), (int32)ESBTickLODLevel::LOD3_BackgroundBatch);
		TestTrue("Has Background tag", StateComp->HasTag(Tags.State_Throttling_Background));
	});

	It("Should register in world subsystem, batch update LODs based on viewer location, and categorize counts", [this]()
	{
		USBDynamicTickManagerSubsystem* Manager = TestWorld->GetSubsystem<USBDynamicTickManagerSubsystem>();
		TestNotNull("Subsystem exists", Manager);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* ActorB = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(3000.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		USceneComponent* ActorBRoot = NewObject<USceneComponent>(ActorB, TEXT("ActorBRoot"));
		ActorB->SetRootComponent(ActorBRoot);
		ActorBRoot->RegisterComponent();
		ActorB->SetActorLocation(FVector(3000.0f, 0.0f, 0.0f));
		USBDynamicTickThrottlingComponent* CompB = NewObject<USBDynamicTickThrottlingComponent>(ActorB, TEXT("CompB"));
		ActorB->AddOwnedComponent(CompB);
		ISBComponentInterface::Execute_OnInitialize(CompB);

		AActor* ActorC = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(25000.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		USceneComponent* ActorCRoot = NewObject<USceneComponent>(ActorC, TEXT("ActorCRoot"));
		ActorC->SetRootComponent(ActorCRoot);
		ActorCRoot->RegisterComponent();
		ActorC->SetActorLocation(FVector(25000.0f, 0.0f, 0.0f));
		USBDynamicTickThrottlingComponent* CompC = NewObject<USBDynamicTickThrottlingComponent>(ActorC, TEXT("CompC"));
		ActorC->AddOwnedComponent(CompC);
		ISBComponentInterface::Execute_OnInitialize(CompC);

		Manager->UpdateAllLODs(FVector::ZeroVector);

		TestEqual("LOD0 count is 1", Manager->GetCountByLOD(ESBTickLODLevel::LOD0_HighPriority), 1);
		TestEqual("LOD1 count is 1", Manager->GetCountByLOD(ESBTickLODLevel::LOD1_MediumPriority), 1);
		TestEqual("LOD3 count is 1", Manager->GetCountByLOD(ESBTickLODLevel::LOD3_BackgroundBatch), 1);

		ActorB->Destroy();
		ActorC->Destroy();
	});
}
