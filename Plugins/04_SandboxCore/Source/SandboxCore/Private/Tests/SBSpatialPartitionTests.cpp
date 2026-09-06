#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Types/SBSpatialPartitionTypes.h"
#include "Components/SBSpatialIndexedComponent.h"
#include "SBCoreTestTypes.h"
#include "Subsystems/SBSpatialPartitionSubsystem.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBSpatialPartitionTestsSpec, "Sandbox.Core.SpatialPartition", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBSpatialIndexedComponent* SpatialComp;
	USBCoreTestStateComponent* StateComp;
END_DEFINE_SPEC(FSBSpatialPartitionTestsSpec)

void FSBSpatialPartitionTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		TestActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		// Sem RootComponent, SetActorLocation nao tem onde gravar e falha em silencio:
		// o ator permanece na origem e a celula espacial nunca muda.
		USceneComponent* TestActorRoot = NewObject<USceneComponent>(TestActor, TEXT("TestActorRoot"));
		TestActor->SetRootComponent(TestActorRoot);
		TestActorRoot->RegisterComponent();

		StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
		TestActor->AddOwnedComponent(StateComp);

		SpatialComp = NewObject<USBSpatialIndexedComponent>(TestActor, TEXT("SpatialComp"));
		TestActor->AddOwnedComponent(SpatialComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(SpatialComp);
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

	It("Should accurately convert 3D world coordinates to spatial cell indices and maintain hash consistency", [this]()
	{
		USBSpatialPartitionSubsystem* Subsystem = TestWorld->GetSubsystem<USBSpatialPartitionSubsystem>();
		TestNotNull("Subsystem exists", Subsystem);

		Subsystem->SetCellSize(1000.0f);

		FSBSpatialCellCoord CellA = Subsystem->WorldToCell(FVector(1500.0f, 2500.0f, 3500.0f));
		TestEqual("CellA X is 1", CellA.X, 1);
		TestEqual("CellA Y is 2", CellA.Y, 2);
		TestEqual("CellA Z is 3", CellA.Z, 3);

		FSBSpatialCellCoord CellB = Subsystem->WorldToCell(FVector(-500.0f, -1500.0f, 50.0f));
		TestEqual("CellB X is -1", CellB.X, -1);
		TestEqual("CellB Y is -2", CellB.Y, -2);
		TestEqual("CellB Z is 0", CellB.Z, 0);

		TestTrue("CellA != CellB", CellA != CellB);
	});

	It("Should register entities and retrieve only nearby elements during radius spatial query", [this]()
	{
		USBSpatialPartitionSubsystem* Subsystem = TestWorld->GetSubsystem<USBSpatialPartitionSubsystem>();
		TestNotNull("Subsystem exists", Subsystem);
		Subsystem->ClearAllEntities();

		FSBSpatialEntityElement E1;
		E1.EntityID = 1;
		E1.EntityType = 1;
		E1.Location = FVector(0.0f, 0.0f, 0.0f);

		FSBSpatialEntityElement E2;
		E2.EntityID = 2;
		E2.EntityType = 1;
		E2.Location = FVector(200.0f, 200.0f, 0.0f);

		FSBSpatialEntityElement E3;
		E3.EntityID = 3;
		E3.EntityType = 1;
		E3.Location = FVector(5000.0f, 5000.0f, 0.0f);

		Subsystem->RegisterEntity(E1);
		Subsystem->RegisterEntity(E2);
		Subsystem->RegisterEntity(E3);

		TArray<FSBSpatialEntityElement> FoundNear = Subsystem->FindEntitiesInRadius(FVector::ZeroVector, 500.0f);
		TestEqual("Found 2 entities within 500 radius", FoundNear.Num(), 2);

		TArray<FSBSpatialEntityElement> FoundAll = Subsystem->FindEntitiesInRadius(FVector::ZeroVector, 10000.0f);
		TestEqual("Found 3 entities within 10000 radius", FoundAll.Num(), 3);

		FSBSpatialGridMetrics Metrics = Subsystem->GetMetrics();
		TestEqual("Total indexed entities is 3", Metrics.TotalIndexedEntities, 3);
	});

	It("Should retrieve accurate entity subset inside bounding box query and support type filtering", [this]()
	{
		USBSpatialPartitionSubsystem* Subsystem = TestWorld->GetSubsystem<USBSpatialPartitionSubsystem>();
		TestNotNull("Subsystem exists", Subsystem);
		Subsystem->ClearAllEntities();

		FSBSpatialEntityElement E1;
		E1.EntityID = 101;
		E1.EntityType = 1;
		E1.Location = FVector(100.0f, 100.0f, 0.0f);

		FSBSpatialEntityElement E2;
		E2.EntityID = 102;
		E2.EntityType = 2;
		E2.Location = FVector(200.0f, 200.0f, 0.0f);

		FSBSpatialEntityElement E3;
		E3.EntityID = 103;
		E3.EntityType = 1;
		E3.Location = FVector(900.0f, 900.0f, 0.0f);

		Subsystem->RegisterEntity(E1);
		Subsystem->RegisterEntity(E2);
		Subsystem->RegisterEntity(E3);

		FBox Box(FVector(0.0f, 0.0f, -100.0f), FVector(300.0f, 300.0f, 100.0f));

		TArray<FSBSpatialEntityElement> InBoxAll = Subsystem->FindEntitiesInBox(Box, 0);
		TestEqual("InBoxAll count is 2", InBoxAll.Num(), 2);

		TArray<FSBSpatialEntityElement> InBoxFiltered = Subsystem->FindEntitiesInBox(Box, 1);
		TestEqual("InBoxFiltered type 1 count is 1", InBoxFiltered.Num(), 1);
		if (InBoxFiltered.Num() > 0)
		{
			TestEqual("InBoxFiltered entity is 101", InBoxFiltered[0].EntityID, 101);
		}
	});

	It("Should auto-register actor with SpatialIndexedComponent, track cell migration, and grant tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		TestActor->SetActorLocation(FVector(100.0f, 100.0f, 0.0f));
		SpatialComp->SetupSpatialEntity(999, 1, 60.0f);

		TestTrue("Has State.Spatial.Indexed tag", StateComp->HasTag(Tags.State_Spatial_Indexed));
		TestTrue("Has State.Spatial.CellActive tag", StateComp->HasTag(Tags.State_Spatial_CellActive));

		TestActor->SetActorLocation(FVector(2500.0f, 3500.0f, 0.0f));
		SpatialComp->SyncLocationToSpatialGrid();

		FSBSpatialCellCoord NewCell = SpatialComp->GetCurrentCellCoord();
		TestEqual("New cell X is 2", NewCell.X, 2);
		TestEqual("New cell Y is 3", NewCell.Y, 3);
	});
}
