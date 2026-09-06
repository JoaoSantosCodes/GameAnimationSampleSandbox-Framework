#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBCoverComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBCoverTestsSpec, "Sandbox.Combat.CoverSystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBCoverComponent* CoverComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBCoverTestsSpec)

void FSBCoverTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CharacterActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(CharacterActor, TEXT("Root"));
		CharacterActor->SetRootComponent(Root);
		Root->RegisterComponent();

		StateComp = NewObject<USBStateComponent>(CharacterActor, TEXT("StateComp"));
		StateComp->RegisterComponent();
		CharacterActor->AddOwnedComponent(StateComp);

		CoverComp = NewObject<USBCoverComponent>(CharacterActor, TEXT("CoverComp"));
		CoverComp->RegisterComponent();
		CharacterActor->AddOwnedComponent(CoverComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(CoverComp);
	});

	AfterEach([this]()
	{
		if (CharacterActor)
		{
			CharacterActor->Destroy();
			CharacterActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should enter low cover and grant cover state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bCoverFired = false;
		ESBCoverType FiredType = ESBCoverType::None;
		CoverComp->OnCoverStateChanged.AddLambda([&bCoverFired, &FiredType](bool bInCover, ESBCoverType Type)
		{
			bCoverFired = bInCover;
			FiredType = Type;
		});

		FSBCoverPoint LowPoint;
		LowPoint.Location = FVector(200.0f, 0.0f, 50.0f);
		LowPoint.Normal = FVector(-1.0f, 0.0f, 0.0f);
		LowPoint.CoverType = ESBCoverType::LowCover;
		LowPoint.bHasTopEdge = true;

		bool bSuccess = CoverComp->EnterCover(LowPoint);

		TestTrue("EnterCover succeeded", bSuccess);
		TestTrue("IsInCover is true", CoverComp->IsInCover());
		TestEqual("CoverType is LowCover", (int32)CoverComp->GetCoverType(), (int32)ESBCoverType::LowCover);
		TestTrue("Has InCover tag", StateComp->HasTag(Tags.State_Combat_InCover));
		TestTrue("Has InCover_Low tag", StateComp->HasTag(Tags.State_Combat_InCover_Low));
		TestFalse("Does not have InCover_High tag", StateComp->HasTag(Tags.State_Combat_InCover_High));
		TestTrue("Delegate fired with true", bCoverFired);
		TestEqual("Delegate fired with LowCover", (int32)FiredType, (int32)ESBCoverType::LowCover);
	});

	It("Should enter high cover and update cover type", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBCoverPoint HighPoint;
		HighPoint.Location = FVector(0.0f, 200.0f, 100.0f);
		HighPoint.CoverType = ESBCoverType::HighCover;

		CoverComp->EnterCover(HighPoint);

		TestTrue("IsInCover is true", CoverComp->IsInCover());
		TestEqual("CoverType is HighCover", (int32)CoverComp->GetCoverType(), (int32)ESBCoverType::HighCover);
		TestTrue("Has InCover tag", StateComp->HasTag(Tags.State_Combat_InCover));
		TestTrue("Has InCover_High tag", StateComp->HasTag(Tags.State_Combat_InCover_High));
		TestFalse("Does not have InCover_Low tag", StateComp->HasTag(Tags.State_Combat_InCover_Low));
	});

	It("Should start and stop peeking at valid edge with tags and delegates", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBCoverPoint Point;
		Point.CoverType = ESBCoverType::HighCover;
		Point.bHasRightEdge = true;
		Point.bHasLeftEdge = false;

		CoverComp->EnterCover(Point);

		// Left edge invalid
		TestFalse("Left peek blocked", CoverComp->StartPeeking(ESBCoverEdge::Left));
		TestFalse("Not peeking", CoverComp->IsPeeking());

		// Right edge valid
		bool bPeekFired = false;
		ESBCoverEdge PeekEdgeFired = ESBCoverEdge::None;
		CoverComp->OnPeekStateChanged.AddLambda([&bPeekFired, &PeekEdgeFired](bool bPeeking, ESBCoverEdge Edge)
		{
			bPeekFired = bPeeking;
			PeekEdgeFired = Edge;
		});

		TestTrue("Right peek succeeded", CoverComp->StartPeeking(ESBCoverEdge::Right));
		TestTrue("IsPeeking is true", CoverComp->IsPeeking());
		TestEqual("Current peek edge is Right", (int32)CoverComp->GetCurrentPeekEdge(), (int32)ESBCoverEdge::Right);
		TestTrue("Has Peeking state tag", StateComp->HasTag(Tags.State_Combat_Peeking));
		TestTrue("Peek delegate fired", bPeekFired);
		TestEqual("Peek edge is Right", (int32)PeekEdgeFired, (int32)ESBCoverEdge::Right);

		// Stop peeking
		CoverComp->StopPeeking();
		TestFalse("IsPeeking is false", CoverComp->IsPeeking());
		TestFalse("Peeking tag removed", StateComp->HasTag(Tags.State_Combat_Peeking));
	});

	It("Should exit cover and clear all cover tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBCoverPoint Point;
		Point.CoverType = ESBCoverType::LowCover;
		Point.bHasTopEdge = true;

		CoverComp->EnterCover(Point);
		CoverComp->StartPeeking(ESBCoverEdge::Top);

		TestTrue("In cover before exit", CoverComp->IsInCover());
		TestTrue("Peeking before exit", CoverComp->IsPeeking());

		CoverComp->ExitCover();

		TestFalse("NotInCover after exit", CoverComp->IsInCover());
		TestFalse("NotPeeking after exit", CoverComp->IsPeeking());
		TestEqual("CoverType is None", (int32)CoverComp->GetCoverType(), (int32)ESBCoverType::None);
		TestFalse("InCover tag removed", StateComp->HasTag(Tags.State_Combat_InCover));
		TestFalse("InCover_Low tag removed", StateComp->HasTag(Tags.State_Combat_InCover_Low));
		TestFalse("Peeking tag removed", StateComp->HasTag(Tags.State_Combat_Peeking));
	});
}
