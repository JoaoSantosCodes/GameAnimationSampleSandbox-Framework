// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBConveyorNetworkComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBConveyorNetworkTestsSpec, "Sandbox.Inventory.ConveyorNetwork", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* Belt1Actor;
	AActor* Belt2Actor;
	AActor* SplitterActor;
	AActor* MergerActor;
	AActor* SorterActor;
	AActor* Out1Actor;
	AActor* Out2Actor;

	USBConveyorNetworkComponent* Belt1Comp;
	USBConveyorNetworkComponent* Belt2Comp;
	USBConveyorNetworkComponent* SplitterComp;
	USBConveyorNetworkComponent* MergerComp;
	USBConveyorNetworkComponent* SorterComp;
	USBConveyorNetworkComponent* Out1Comp;
	USBConveyorNetworkComponent* Out2Comp;

	USBStateComponent* Belt1StateComp;
	USBStateComponent* Belt2StateComp;
	USBStateComponent* SplitterStateComp;
	USBStateComponent* MergerStateComp;
	USBStateComponent* SorterStateComp;
	USBStateComponent* Out1StateComp;
	USBStateComponent* Out2StateComp;
END_DEFINE_SPEC(FSBConveyorNetworkTestsSpec)

void FSBConveyorNetworkTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Belt 1
		Belt1Actor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Belt1Root = NewObject<USceneComponent>(Belt1Actor, TEXT("Belt1Root"));
		Belt1Actor->SetRootComponent(Belt1Root);
		Belt1Root->RegisterComponent();
		Belt1StateComp = NewObject<USBStateComponent>(Belt1Actor, TEXT("Belt1StateComp"));
		Belt1Actor->AddOwnedComponent(Belt1StateComp);
		Belt1Comp = NewObject<USBConveyorNetworkComponent>(Belt1Actor, TEXT("Belt1Comp"));
		Belt1Actor->AddOwnedComponent(Belt1Comp);

		// Belt 2
		Belt2Actor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(300.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Belt2Root = NewObject<USceneComponent>(Belt2Actor, TEXT("Belt2Root"));
		Belt2Actor->SetRootComponent(Belt2Root);
		Belt2Root->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		Belt2Actor->SetActorLocation(FVector(300.0f, 0.0f, 0.0f));
		Belt2StateComp = NewObject<USBStateComponent>(Belt2Actor, TEXT("Belt2StateComp"));
		Belt2Actor->AddOwnedComponent(Belt2StateComp);
		Belt2Comp = NewObject<USBConveyorNetworkComponent>(Belt2Actor, TEXT("Belt2Comp"));
		Belt2Actor->AddOwnedComponent(Belt2Comp);

		// Splitter
		SplitterActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(600.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* SplitterRoot = NewObject<USceneComponent>(SplitterActor, TEXT("SplitterRoot"));
		SplitterActor->SetRootComponent(SplitterRoot);
		SplitterRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		SplitterActor->SetActorLocation(FVector(600.0f, 0.0f, 0.0f));
		SplitterStateComp = NewObject<USBStateComponent>(SplitterActor, TEXT("SplitterStateComp"));
		SplitterActor->AddOwnedComponent(SplitterStateComp);
		SplitterComp = NewObject<USBConveyorNetworkComponent>(SplitterActor, TEXT("SplitterComp"));
		SplitterActor->AddOwnedComponent(SplitterComp);

		// Merger
		MergerActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(900.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* MergerRoot = NewObject<USceneComponent>(MergerActor, TEXT("MergerRoot"));
		MergerActor->SetRootComponent(MergerRoot);
		MergerRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		MergerActor->SetActorLocation(FVector(900.0f, 0.0f, 0.0f));
		MergerStateComp = NewObject<USBStateComponent>(MergerActor, TEXT("MergerStateComp"));
		MergerActor->AddOwnedComponent(MergerStateComp);
		MergerComp = NewObject<USBConveyorNetworkComponent>(MergerActor, TEXT("MergerComp"));
		MergerActor->AddOwnedComponent(MergerComp);

		// Sorter
		SorterActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(1200.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* SorterRoot = NewObject<USceneComponent>(SorterActor, TEXT("SorterRoot"));
		SorterActor->SetRootComponent(SorterRoot);
		SorterRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		SorterActor->SetActorLocation(FVector(1200.0f, 0.0f, 0.0f));
		SorterStateComp = NewObject<USBStateComponent>(SorterActor, TEXT("SorterStateComp"));
		SorterActor->AddOwnedComponent(SorterStateComp);
		SorterComp = NewObject<USBConveyorNetworkComponent>(SorterActor, TEXT("SorterComp"));
		SorterActor->AddOwnedComponent(SorterComp);

		// Out 1
		Out1Actor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(1500.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Out1Root = NewObject<USceneComponent>(Out1Actor, TEXT("Out1Root"));
		Out1Actor->SetRootComponent(Out1Root);
		Out1Root->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		Out1Actor->SetActorLocation(FVector(1500.0f, 0.0f, 0.0f));
		Out1StateComp = NewObject<USBStateComponent>(Out1Actor, TEXT("Out1StateComp"));
		Out1Actor->AddOwnedComponent(Out1StateComp);
		Out1Comp = NewObject<USBConveyorNetworkComponent>(Out1Actor, TEXT("Out1Comp"));
		Out1Actor->AddOwnedComponent(Out1Comp);

		// Out 2
		Out2Actor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(1800.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Out2Root = NewObject<USceneComponent>(Out2Actor, TEXT("Out2Root"));
		Out2Actor->SetRootComponent(Out2Root);
		Out2Root->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		Out2Actor->SetActorLocation(FVector(1800.0f, 0.0f, 0.0f));
		Out2StateComp = NewObject<USBStateComponent>(Out2Actor, TEXT("Out2StateComp"));
		Out2Actor->AddOwnedComponent(Out2StateComp);
		Out2Comp = NewObject<USBConveyorNetworkComponent>(Out2Actor, TEXT("Out2Comp"));
		Out2Actor->AddOwnedComponent(Out2Comp);

		ISBComponentInterface::Execute_OnInitialize(Belt1StateComp);
		ISBComponentInterface::Execute_OnInitialize(Belt1Comp);
		ISBComponentInterface::Execute_OnInitialize(Belt2StateComp);
		ISBComponentInterface::Execute_OnInitialize(Belt2Comp);
		ISBComponentInterface::Execute_OnInitialize(SplitterStateComp);
		ISBComponentInterface::Execute_OnInitialize(SplitterComp);
		ISBComponentInterface::Execute_OnInitialize(MergerStateComp);
		ISBComponentInterface::Execute_OnInitialize(MergerComp);
		ISBComponentInterface::Execute_OnInitialize(SorterStateComp);
		ISBComponentInterface::Execute_OnInitialize(SorterComp);
		ISBComponentInterface::Execute_OnInitialize(Out1StateComp);
		ISBComponentInterface::Execute_OnInitialize(Out1Comp);
		ISBComponentInterface::Execute_OnInitialize(Out2StateComp);
		ISBComponentInterface::Execute_OnInitialize(Out2Comp);
	});

	AfterEach([this]()
	{
		if (Out2Actor) { Out2Actor->Destroy(); Out2Actor = nullptr; }
		if (Out1Actor) { Out1Actor->Destroy(); Out1Actor = nullptr; }
		if (SorterActor) { SorterActor->Destroy(); SorterActor = nullptr; }
		if (MergerActor) { MergerActor->Destroy(); MergerActor = nullptr; }
		if (SplitterActor) { SplitterActor->Destroy(); SplitterActor = nullptr; }
		if (Belt2Actor) { Belt2Actor->Destroy(); Belt2Actor = nullptr; }
		if (Belt1Actor) { Belt1Actor->Destroy(); Belt1Actor = nullptr; }

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should enqueue item, advance belt progress, and transfer to downstream conveyor", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		Belt1Comp->SetupNode(ESBConveyorNodeType::BeltSegment, 300.0f, 4);
		Belt2Comp->SetupNode(ESBConveyorNodeType::BeltSegment, 300.0f, 4);

		Belt1Comp->ConnectOutput(Belt2Comp);
		Belt1Comp->EnqueueItem(FName(TEXT("IronOre")), 5, FGameplayTag::EmptyTag);

		TestEqual("Belt1 state is Conveying", (int32)Belt1Comp->GetConveyorState(), (int32)ESBConveyorState::Conveying);
		TestTrue("Belt1 has Conveying tag", Belt1StateComp->HasTag(Tags.State_Logistics_Conveying));

		// Tick 1.0s (300cm at 300cm/s -> progress reaches 1.0 -> transfers to Belt2)
		Belt1Comp->SimulateConveyorTick(1.0f);

		TestEqual("Belt1 is now empty", Belt1Comp->GetItemQueue().Num(), 0);
		TestEqual("Belt2 received item", Belt2Comp->GetItemQueue().Num(), 1);
		TestEqual("Belt2 item ID is IronOre", Belt2Comp->GetItemQueue()[0].ItemId, FName(TEXT("IronOre")));
		TestEqual("Belt2 item quantity is 5", Belt2Comp->GetItemQueue()[0].Quantity, 5);
	});

	It("Should split item stream evenly between two outputs in round-robin fashion", [this]()
	{
		SplitterComp->SetupNode(ESBConveyorNodeType::Splitter, 300.0f, 4);
		Out1Comp->SetupNode(ESBConveyorNodeType::BeltSegment, 300.0f, 4);
		Out2Comp->SetupNode(ESBConveyorNodeType::BeltSegment, 300.0f, 4);

		SplitterComp->ConnectOutput(Out1Comp);
		SplitterComp->ConnectOutput(Out2Comp);

		SplitterComp->EnqueueItem(FName(TEXT("ItemA")), 1, FGameplayTag::EmptyTag);
		SplitterComp->EnqueueItem(FName(TEXT("ItemB")), 1, FGameplayTag::EmptyTag);

		// First item goes to Out1
		SplitterComp->SimulateConveyorTick(1.0f);
		TestEqual("Out1 received ItemA", Out1Comp->GetItemQueue().Num(), 1);
		TestEqual("Out1 item is ItemA", Out1Comp->GetItemQueue()[0].ItemId, FName(TEXT("ItemA")));

		// Second item goes to Out2
		SplitterComp->SimulateConveyorTick(1.0f);
		TestEqual("Out2 received ItemB", Out2Comp->GetItemQueue().Num(), 1);
		TestEqual("Out2 item is ItemB", Out2Comp->GetItemQueue()[0].ItemId, FName(TEXT("ItemB")));
	});

	It("Should merge items from two input conveyors into single output and grant Merging tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		Belt1Comp->SetupNode(ESBConveyorNodeType::BeltSegment, 300.0f, 4);
		Belt2Comp->SetupNode(ESBConveyorNodeType::BeltSegment, 300.0f, 4);
		MergerComp->SetupNode(ESBConveyorNodeType::Merger, 300.0f, 4);

		Belt1Comp->ConnectOutput(MergerComp);
		Belt2Comp->ConnectOutput(MergerComp);

		TestEqual("Merger state is Merging", (int32)MergerComp->GetConveyorState(), (int32)ESBConveyorState::Merging);
		TestTrue("Merger has Merging tag", MergerStateComp->HasTag(Tags.State_Logistics_Merging));

		Belt1Comp->EnqueueItem(FName(TEXT("CopperOre")), 2, FGameplayTag::EmptyTag);
		Belt2Comp->EnqueueItem(FName(TEXT("Coal")), 3, FGameplayTag::EmptyTag);

		Belt1Comp->SimulateConveyorTick(1.0f);
		Belt2Comp->SimulateConveyorTick(1.0f);

		TestEqual("Merger holds both items", MergerComp->GetItemQueue().Num(), 2);
	});

	It("Should route filtered item to primary output and jam conveyor on full downstream buffer", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SorterComp->SetupNode(ESBConveyorNodeType::SmartSorter, 300.0f, 4);
		Out1Comp->SetupNode(ESBConveyorNodeType::BeltSegment, 300.0f, 2); // Capacity 2
		Out2Comp->SetupNode(ESBConveyorNodeType::BeltSegment, 300.0f, 4);

		SorterComp->ConnectOutput(Out1Comp);
		SorterComp->ConnectOutput(Out2Comp);

		SorterComp->SetFilterTag(Tags.State_Logistics_Sorting);

		TestEqual("Sorter state is Sorting", (int32)SorterComp->GetConveyorState(), (int32)ESBConveyorState::Sorting);
		TestTrue("Sorter has Sorting tag", SorterStateComp->HasTag(Tags.State_Logistics_Sorting));

		// Fill Out1Comp to maximum capacity
		Out1Comp->EnqueueItem(FName(TEXT("Block1")), 1, FGameplayTag::EmptyTag);
		Out1Comp->EnqueueItem(FName(TEXT("Block2")), 1, FGameplayTag::EmptyTag);

		// Enqueue matched item on Sorter
		SorterComp->EnqueueItem(FName(TEXT("GoldOre")), 1, Tags.State_Logistics_Sorting);
		SorterComp->SimulateConveyorTick(1.0f);

		TestTrue("Sorter is jammed due to full downstream buffer", SorterComp->IsJammed());
		TestEqual("Sorter state is Jammed", (int32)SorterComp->GetConveyorState(), (int32)ESBConveyorState::Jammed);
		TestTrue("Sorter has Jammed tag", SorterStateComp->HasTag(Tags.State_Logistics_Jammed));
		TestEqual("GoldOre retained in Sorter without data loss", SorterComp->GetItemQueue().Num(), 1);
	});
}
