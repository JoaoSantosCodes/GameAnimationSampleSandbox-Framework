// Copyright 2026 João Santos. All Rights Reserved.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Subsystems/SBSandboxPersistenceSubsystem.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Components/SBPersistenceComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSBPersistenceSubsystemTest, "Sandbox.Persistence.SubsystemVerification", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSBPersistenceSubsystemTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	GameInstance->InitializeStandalone();
	World->InitializeActorsForPlay(FURL());

	USBSandboxPersistenceSubsystem* PersistenceSubsystem = World->GetSubsystem<USBSandboxPersistenceSubsystem>();
	UTEST_NOT_NULL(TEXT("PersistenceSubsystem should be valid"), PersistenceSubsystem);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
	UTEST_NOT_NULL(TEXT("TestActor should be spawned successfully"), TestActor);

	// Sem RootComponent, SetActorTransform nao tem onde gravar e falha em silencio:
	// o transform salvo seria sempre a identidade.
	USceneComponent* TestActorRoot = NewObject<USceneComponent>(TestActor, TEXT("TestActorRoot"));
	TestActor->SetRootComponent(TestActorRoot);
	TestActorRoot->RegisterComponent();

	USBPersistenceComponent* PersistenceComp = NewObject<USBPersistenceComponent>(TestActor);
	UTEST_NOT_NULL(TEXT("PersistenceComp should be created"), PersistenceComp);
	PersistenceComp->RegisterComponent();

	FGuid TestGuid = FGuid::NewGuid();
	PersistenceComp->PersistentId.Guid = TestGuid;

	FTransform TestTransform(FRotator(0.f, 90.f, 0.f), FVector(100.f, 200.f, 300.f));
	TestActor->SetActorTransform(TestTransform);

	USBSavePayload* SavePayload = NewObject<USBSavePayload>(GameInstance);
	PersistenceSubsystem->SaveWorldState(SavePayload);

	TestActor->SetActorTransform(FTransform::Identity);
	UTEST_EQUAL(TEXT("Transform should be reset to Identity"), TestActor->GetActorTransform().GetLocation(), FVector::ZeroVector);

	PersistenceSubsystem->LoadWorldState(SavePayload);
	UTEST_EQUAL(TEXT("Transform Location should be restored"), TestActor->GetActorTransform().GetLocation(), FVector(100.f, 200.f, 300.f));
	UTEST_EQUAL(TEXT("Transform Rotation should be restored"), TestActor->GetActorTransform().GetRotation().Rotator().Yaw, 90.0);

	PersistenceSubsystem->RecordActorDestruction(TestGuid);
	PersistenceSubsystem->SaveWorldState(SavePayload);

	PersistenceSubsystem->LoadWorldState(SavePayload);
	UTEST_FALSE(TEXT("TestActor should be destroyed/invalid"), IsValid(TestActor));

	GameInstance->Shutdown();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(true);

	return true;
}
