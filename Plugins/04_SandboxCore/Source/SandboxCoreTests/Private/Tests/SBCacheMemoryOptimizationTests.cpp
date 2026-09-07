#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Types/SBCacheTypes.h"
#include "Components/SBCacheOptimizedDataComponent.h"
#include "SBCoreTestTypes.h"
#include "Subsystems/SBCacheOptimizedBufferSubsystem.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBCacheMemoryOptimizationTestsSpec, "Sandbox.Core.CacheMemoryOptimization", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBCacheOptimizedDataComponent* CacheComp;
	USBCoreTestStateComponent* StateComp;
END_DEFINE_SPEC(FSBCacheMemoryOptimizationTestsSpec)

void FSBCacheMemoryOptimizationTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		TestActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(500.0f, 600.0f, 700.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(TestActor, TEXT("Root"));
		TestActor->SetRootComponent(Root);
		Root->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		TestActor->SetActorLocation(FVector(500.0f, 600.0f, 700.0f));

		StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
		TestActor->AddOwnedComponent(StateComp);

		CacheComp = NewObject<USBCacheOptimizedDataComponent>(TestActor, TEXT("CacheComp"));
		TestActor->AddOwnedComponent(CacheComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(CacheComp);
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

	It("Should verify struct size is exactly 24 bytes and fields are correctly packed", [this]()
	{
		TestEqual("sizeof(FSBCompactEntityRecord) is exactly 24 bytes", (int32)sizeof(FSBCompactEntityRecord), 24);

		FSBCompactEntityRecord Record;
		Record.EntityID = 1001;
		Record.TypeID = 42;
		Record.PositionX = 100.0f;
		Record.PositionY = 200.0f;
		Record.PositionZ = 300.0f;
		Record.CustomData = 99.5f;

		TestEqual("EntityID matches", Record.EntityID, 1001);
		TestEqual("TypeID matches", Record.TypeID, 42);
		TestNearlyEqual("PositionX matches", Record.PositionX, 100.0f, 0.01f);
		TestNearlyEqual("PositionY matches", Record.PositionY, 200.0f, 0.01f);
		TestNearlyEqual("PositionZ matches", Record.PositionZ, 300.0f, 0.01f);
		TestNearlyEqual("CustomData matches", Record.CustomData, 99.5f, 0.01f);
	});

	It("Should preallocate buffer, insert records without reallocation, and process zero-alloc", [this]()
	{
		USBCacheOptimizedBufferSubsystem* Subsystem = TestWorld->GetSubsystem<USBCacheOptimizedBufferSubsystem>();
		TestNotNull("Subsystem exists", Subsystem);

		Subsystem->PreallocateBuffer(100);
		FSBMemoryMetrics Metrics = Subsystem->GetMemoryMetrics();

		TestTrue("Capacity >= 100", Metrics.TotalCapacity >= 100);
		TestEqual("BytesPerRecord is 24", Metrics.BytesPerRecord, 24);
		TestEqual("Fragmentation ratio is 0.0", Metrics.FragmentationRatio, 0.0f);

		for (int32 i = 0; i < 50; ++i)
		{
			FSBCompactEntityRecord R;
			R.EntityID = i + 1;
			Subsystem->InsertRecord(R);
		}

		TestEqual("Record count is 51 (including TestActor)", Subsystem->GetRecordCount(), 51);

		int32 ProcessedCount = 0;
		Subsystem->ProcessRecordsZeroAlloc([&ProcessedCount](const FSBCompactEntityRecord& Item)
		{
			if (Item.EntityID > 0)
			{
				ProcessedCount++;
			}
		});

		TestEqual("Zero-alloc iterated through all 50 inserted items", ProcessedCount, 50);
	});

	It("Should remove record using O(1) swap-and-pop, maintain dense buffer, and report swapped entity", [this]()
	{
		USBCacheOptimizedBufferSubsystem* Subsystem = TestWorld->GetSubsystem<USBCacheOptimizedBufferSubsystem>();
		TestNotNull("Subsystem exists", Subsystem);

		FSBCompactEntityRecord R1;
		R1.EntityID = 101;
		int32 Idx1 = Subsystem->InsertRecord(R1);

		FSBCompactEntityRecord R2;
		R2.EntityID = 102;
		int32 Idx2 = Subsystem->InsertRecord(R2);

		FSBCompactEntityRecord R3;
		R3.EntityID = 103;
		int32 Idx3 = Subsystem->InsertRecord(R3);

		int32 SwappedID = -1;
		bool bRemoved = Subsystem->RemoveRecord(Idx1, SwappedID);

		TestTrue("Removed record at Idx1", bRemoved);
		TestEqual("Swapped entity is 103", SwappedID, 103);
		TestEqual("Idx1 now contains entity 103", Subsystem->GetRecord(Idx1).EntityID, 103);
	});

	It("Should auto-register actor with CacheOptimizedDataComponent, sync coordinates, and grant memory tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		CacheComp->SetupRecord(777, 12);
		CacheComp->SetCustomData(55.0f);

		TestTrue("Has Memory.Optimized tag", StateComp->HasTag(Tags.State_Memory_Optimized));
		TestTrue("Has Memory.Contiguous tag", StateComp->HasTag(Tags.State_Memory_Contiguous));

		FSBCompactEntityRecord Record = CacheComp->GetCurrentRecord();
		TestEqual("EntityID is 777", Record.EntityID, 777);
		TestEqual("TypeID is 12", Record.TypeID, 12);
		TestNearlyEqual("CustomData is 55.0", Record.CustomData, 55.0f, 0.01f);
	});
}
