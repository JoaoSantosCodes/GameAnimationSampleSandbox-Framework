// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Types/SBThreadingTypes.h"
#include "Components/SBAsyncParallelDataComponent.h"
#include "SBCoreTestTypes.h"
#include "Subsystems/SBAsyncTaskManagerSubsystem.h"
#include "Async/TaskGraphInterfaces.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBAsyncThreadingTestsSpec, "Sandbox.Core.AsyncThreading", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBAsyncParallelDataComponent* AsyncDataComp;
	USBCoreTestStateComponent* StateComp;
END_DEFINE_SPEC(FSBAsyncThreadingTestsSpec)

void FSBAsyncThreadingTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		TestActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		// Um AActor puro nao tem RootComponent: sem ele, SetActorLocation, SetActorTransform
		// e TeleportTo falham em silencio e o ator fica preso na origem.
		USceneComponent* TestActorRoot = NewObject<USceneComponent>(TestActor, TEXT("TestActorRoot"));
		TestActor->SetRootComponent(TestActorRoot);
		TestActorRoot->RegisterComponent();

		StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
		TestActor->AddOwnedComponent(StateComp);

		AsyncDataComp = NewObject<USBAsyncParallelDataComponent>(TestActor, TEXT("AsyncDataComp"));
		TestActor->AddOwnedComponent(AsyncDataComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(AsyncDataComp);
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

	It("Should process 1,000 vectors concurrently with ParallelFor and verify correct translations", [this]()
	{
		USBAsyncTaskManagerSubsystem* Subsystem = TestWorld->GetSubsystem<USBAsyncTaskManagerSubsystem>();
		TestNotNull("Subsystem exists", Subsystem);

		TArray<FVector> Vectors;
		Vectors.SetNum(1000);
		for (int32 i = 0; i < 1000; ++i)
		{
			Vectors[i] = FVector(i * 1.0f, i * 2.0f, i * 3.0f);
		}

		Subsystem->DispatchParallelTransformBatch(Vectors, FVector(10.0f, 20.0f, 30.0f));

		TestNearlyEqual("Vector 0 X translated", Vectors[0].X, 10.0, 0.01);
		TestNearlyEqual("Vector 0 Y translated", Vectors[0].Y, 20.0, 0.01);
		TestNearlyEqual("Vector 0 Z translated", Vectors[0].Z, 30.0, 0.01);

		TestNearlyEqual("Vector 999 X translated", Vectors[999].X, 1009.0, 0.01);
		TestNearlyEqual("Vector 999 Y translated", Vectors[999].Y, 2018.0, 0.01);
		TestNearlyEqual("Vector 999 Z translated", Vectors[999].Z, 3027.0, 0.01);

		TestTrue("Completed tasks counter incremented", Subsystem->GetTotalCompletedTasks() > 0);
	});

	It("Should dispatch async background calculation, process elements, and update completion metrics", [this]()
	{
		USBAsyncTaskManagerSubsystem* Subsystem = TestWorld->GetSubsystem<USBAsyncTaskManagerSubsystem>();
		TestNotNull("Subsystem exists", Subsystem);

		TArray<float> Inputs;
		Inputs.Add(10.0f);
		Inputs.Add(20.0f);
		Inputs.Add(30.0f);
		Inputs.Add(40.0f);

		TArray<float> AsyncOutputs;
		bool bFinished = false;

		Subsystem->DispatchAsyncBatchCalculation(
			Inputs,
			[](float V) -> float
			{
				return (V * 2.0f) + 5.0f;
			},
			[&AsyncOutputs, &bFinished](const TArray<float>& Results)
			{
				AsyncOutputs = Results;
				bFinished = true;
			}
		);

		// Allow background thread to process and flush game thread task
		FPlatformProcess::Sleep(0.05f);
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);

		TestTrue("Async batch completed", bFinished);
		TestEqual("Output array has 4 items", AsyncOutputs.Num(), 4);
		if (AsyncOutputs.Num() == 4)
		{
			TestNearlyEqual("Outputs[0] is (10*2)+5 = 25.0", AsyncOutputs[0], 25.0f, 0.01f);
			TestNearlyEqual("Outputs[1] is (20*2)+5 = 45.0", AsyncOutputs[1], 45.0f, 0.01f);
			TestNearlyEqual("Outputs[2] is (30*2)+5 = 65.0", AsyncOutputs[2], 65.0f, 0.01f);
			TestNearlyEqual("Outputs[3] is (40*2)+5 = 85.0", AsyncOutputs[3], 85.0f, 0.01f);
		}
	});

	It("Should maintain front buffer stability during back buffer update and swap atomically on commit", [this]()
	{
		AsyncDataComp->SetFrontBufferValue(100.0f);
		TestNearlyEqual("Front buffer is 100.0", AsyncDataComp->GetFrontBufferValue(), 100.0f, 0.01f);
		TestNearlyEqual("Back buffer is 100.0", AsyncDataComp->GetBackBufferValue(), 100.0f, 0.01f);

		AsyncDataComp->RequestAsyncCalculation(3.0f, 50.0f);

		FPlatformProcess::Sleep(0.05f);
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);

		TestNearlyEqual("Front buffer remains stable at 100.0 before commit", AsyncDataComp->GetFrontBufferValue(), 100.0f, 0.01f);
		TestNearlyEqual("Back buffer computed to 350.0", AsyncDataComp->GetBackBufferValue(), 350.0f, 0.01f);

		AsyncDataComp->CommitBackBuffer();
		TestNearlyEqual("Front buffer swapped to 350.0 after commit", AsyncDataComp->GetFrontBufferValue(), 350.0f, 0.01f);
	});

	It("Should grant State.Async.DoubleBufferActive and State.Async.WorkCompleted tags to actor", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		TestTrue("StateComp has State.Async.DoubleBufferActive", StateComp->HasTag(Tags.State_Async_DoubleBufferActive));
		TestTrue("StateComp has State.Async.WorkCompleted", StateComp->HasTag(Tags.State_Async_WorkCompleted));
		TestFalse("StateComp does not have State.Async.TaskRunning", StateComp->HasTag(Tags.State_Async_TaskRunning));
	});
}
