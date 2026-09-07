// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBStructuralIntegrityComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBStructuralIntegrityTestsSpec, "Sandbox.Inventory.StructuralIntegrity", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* FoundationActor;
	AActor* WallActor;
	AActor* CeilingActor;
	USBStructuralIntegrityComponent* FoundationComp;
	USBStructuralIntegrityComponent* WallComp;
	USBStructuralIntegrityComponent* CeilingComp;
	USBStateComponent* FoundationStateComp;
	USBStateComponent* WallStateComp;
	USBStateComponent* CeilingStateComp;
END_DEFINE_SPEC(FSBStructuralIntegrityTestsSpec)

void FSBStructuralIntegrityTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Foundation
		FoundationActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* FRoot = NewObject<USceneComponent>(FoundationActor, TEXT("FRoot"));
		FoundationActor->SetRootComponent(FRoot);
		FRoot->RegisterComponent();

		FoundationStateComp = NewObject<USBStateComponent>(FoundationActor, TEXT("FoundationStateComp"));
		FoundationActor->AddOwnedComponent(FoundationStateComp);

		FoundationComp = NewObject<USBStructuralIntegrityComponent>(FoundationActor, TEXT("FoundationComp"));
		FoundationActor->AddOwnedComponent(FoundationComp);

		// Wall
		WallActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(0.0f, 0.0f, 300.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* WRoot = NewObject<USceneComponent>(WallActor, TEXT("WRoot"));
		WallActor->SetRootComponent(WRoot);
		WRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		WallActor->SetActorLocation(FVector(0.0f, 0.0f, 300.0f));

		WallStateComp = NewObject<USBStateComponent>(WallActor, TEXT("WallStateComp"));
		WallActor->AddOwnedComponent(WallStateComp);

		WallComp = NewObject<USBStructuralIntegrityComponent>(WallActor, TEXT("WallComp"));
		WallActor->AddOwnedComponent(WallComp);

		// Ceiling
		CeilingActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(300.0f, 0.0f, 300.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* CRoot = NewObject<USceneComponent>(CeilingActor, TEXT("CRoot"));
		CeilingActor->SetRootComponent(CRoot);
		CRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		CeilingActor->SetActorLocation(FVector(300.0f, 0.0f, 300.0f));

		CeilingStateComp = NewObject<USBStateComponent>(CeilingActor, TEXT("CeilingStateComp"));
		CeilingActor->AddOwnedComponent(CeilingStateComp);

		CeilingComp = NewObject<USBStructuralIntegrityComponent>(CeilingActor, TEXT("CeilingComp"));
		CeilingActor->AddOwnedComponent(CeilingComp);

		ISBComponentInterface::Execute_OnInitialize(FoundationStateComp);
		ISBComponentInterface::Execute_OnInitialize(FoundationComp);
		ISBComponentInterface::Execute_OnInitialize(WallStateComp);
		ISBComponentInterface::Execute_OnInitialize(WallComp);
		ISBComponentInterface::Execute_OnInitialize(CeilingStateComp);
		ISBComponentInterface::Execute_OnInitialize(CeilingComp);
	});

	AfterEach([this]()
	{
		if (CeilingActor)
		{
			CeilingActor->Destroy();
			CeilingActor = nullptr;
		}

		if (WallActor)
		{
			WallActor->Destroy();
			WallActor = nullptr;
		}

		if (FoundationActor)
		{
			FoundationActor->Destroy();
			FoundationActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should initialize foundation piece as ground anchor, grant Anchor and Supported tags, and report 100% stability", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FoundationComp->SetGroundAnchor(true);

		TestTrue("Foundation is supported", FoundationComp->IsSupported());
		TestEqual("Foundation distance is 0", FoundationComp->GetStructuralData().DistanceFromAnchor, 0);
		TestEqual("Foundation stability is 100", FoundationComp->GetStructuralData().StructuralStability, 100.0f);
		TestEqual("Foundation state is Stable", (int32)FoundationComp->GetStabilityState(), (int32)ESBStructuralStabilityState::Stable);
		TestTrue("Foundation has Anchor tag", FoundationStateComp->HasTag(Tags.State_Building_Anchor));
		TestTrue("Foundation has Supported tag", FoundationStateComp->HasTag(Tags.State_Building_Supported));
	});

	It("Should connect wall and ceiling pieces to foundation, propagate distance, and grant Supported tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FoundationComp->SetGroundAnchor(true);
		WallComp->RegisterNeighborPiece(FoundationComp);
		CeilingComp->RegisterNeighborPiece(WallComp);

		TestTrue("Wall is supported", WallComp->IsSupported());
		TestEqual("Wall distance is 1", WallComp->GetStructuralData().DistanceFromAnchor, 1);
		TestTrue("Wall has Supported tag", WallStateComp->HasTag(Tags.State_Building_Supported));

		TestTrue("Ceiling is supported", CeilingComp->IsSupported());
		TestEqual("Ceiling distance is 2", CeilingComp->GetStructuralData().DistanceFromAnchor, 2);
		TestTrue("Ceiling has Supported tag", CeilingStateComp->HasTag(Tags.State_Building_Supported));
	});

	It("Should apply heavy load weight exceeding stress threshold, trigger Stressed state, and grant Stressed tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FoundationComp->SetGroundAnchor(true);
		WallComp->RegisterNeighborPiece(FoundationComp);
		CeilingComp->RegisterNeighborPiece(WallComp);

		CeilingComp->AddSupportedLoad(800.0f);

		TestEqual("Ceiling state is Stressed", (int32)CeilingComp->GetStabilityState(), (int32)ESBStructuralStabilityState::Stressed);
		TestTrue("Ceiling has Stressed tag", CeilingStateComp->HasTag(Tags.State_Building_Stressed));
	});

	It("Should trigger collapse when foundation is destroyed/unanchored, propagate collapse in cascade, and grant Collapsing tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FoundationComp->SetGroundAnchor(true);
		WallComp->RegisterNeighborPiece(FoundationComp);
		CeilingComp->RegisterNeighborPiece(WallComp);

		FoundationComp->SetGroundAnchor(false);

		TestFalse("Foundation lost support", FoundationComp->IsSupported());
		TestFalse("Wall lost support", WallComp->IsSupported());
		TestFalse("Ceiling lost support", CeilingComp->IsSupported());

		TestEqual("Ceiling state is Collapsing", (int32)CeilingComp->GetStabilityState(), (int32)ESBStructuralStabilityState::Collapsing);
		TestTrue("Ceiling has Collapsing tag", CeilingStateComp->HasTag(Tags.State_Building_Collapsing));
		TestFalse("Ceiling lost Supported tag", CeilingStateComp->HasTag(Tags.State_Building_Supported));
	});
}
