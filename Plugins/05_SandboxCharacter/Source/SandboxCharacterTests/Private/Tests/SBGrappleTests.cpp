// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBGrappleComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBGrappleTestsSpec, "Sandbox.Character.GrapplingHookAndSwing", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBGrappleComponent* GrappleComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBGrappleTestsSpec)

void FSBGrappleTestsSpec::Define()
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
		CharacterActor->AddOwnedComponent(StateComp);

		GrappleComp = NewObject<USBGrappleComponent>(CharacterActor, TEXT("GrappleComp"));
		CharacterActor->AddOwnedComponent(GrappleComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(GrappleComp);
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

	It("Should attach anchor point, set swinging state, and apply Grappling state tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bAnchoredFired = false;
		FVector AnchoredLoc = FVector::ZeroVector;
		GrappleComp->OnGrappleAnchored.AddLambda([&bAnchoredFired, &AnchoredLoc](const FVector& Loc)
		{
			bAnchoredFired = true;
			AnchoredLoc = Loc;
		});

		const FVector TargetAnchor(500.0f, 0.0f, 500.0f);
		bool bAttached = GrappleComp->AttachAnchorPoint(TargetAnchor, FVector::UpVector);

		TestTrue("Attach succeeded", bAttached);
		TestTrue("Is attached", GrappleComp->IsAttached());
		TestEqual("State is Swinging", (int32)GrappleComp->GetGrappleState(), (int32)ESBGrappleState::Swinging);
		TestTrue("Has Grappling tag", StateComp->HasTag(Tags.State_Movement_Grappling));
		TestTrue("Has Swinging tag", StateComp->HasTag(Tags.State_Movement_Grappling_Swinging));
		TestFalse("Does not have Pulling tag", StateComp->HasTag(Tags.State_Movement_Grappling_Pulling));
		TestTrue("Anchor delegate fired", bAnchoredFired);
		TestEqual("Anchor location matches", AnchoredLoc, TargetAnchor);
	});

	It("Should transition to winch pull mode and update pulling state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		const FVector TargetAnchor(500.0f, 0.0f, 500.0f);
		GrappleComp->AttachAnchorPoint(TargetAnchor, FVector::UpVector);

		bool bPullStarted = GrappleComp->StartPull();

		TestTrue("Start pull succeeded", bPullStarted);
		TestTrue("Is pulling", GrappleComp->IsPulling());
		TestEqual("State is Pulling", (int32)GrappleComp->GetGrappleState(), (int32)ESBGrappleState::Pulling);
		TestTrue("Has Grappling tag", StateComp->HasTag(Tags.State_Movement_Grappling));
		TestTrue("Has Pulling tag", StateComp->HasTag(Tags.State_Movement_Grappling_Pulling));
		TestFalse("Does not have Swinging tag", StateComp->HasTag(Tags.State_Movement_Grappling_Swinging));
	});

	It("Should transition back to swinging mode and update swinging state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		const FVector TargetAnchor(500.0f, 0.0f, 500.0f);
		GrappleComp->AttachAnchorPoint(TargetAnchor, FVector::UpVector);
		GrappleComp->StartPull();

		bool bSwingStarted = GrappleComp->StartSwing();

		TestTrue("Start swing succeeded", bSwingStarted);
		TestTrue("Is swinging", GrappleComp->IsSwinging());
		TestEqual("State is Swinging", (int32)GrappleComp->GetGrappleState(), (int32)ESBGrappleState::Swinging);
		TestTrue("Has Grappling tag", StateComp->HasTag(Tags.State_Movement_Grappling));
		TestTrue("Has Swinging tag", StateComp->HasTag(Tags.State_Movement_Grappling_Swinging));
		TestFalse("Does not have Pulling tag", StateComp->HasTag(Tags.State_Movement_Grappling_Pulling));
	});

	It("Should release anchor point, calculate launch impulse, and clear grapple tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		const FVector TargetAnchor(500.0f, 0.0f, 500.0f);
		GrappleComp->AttachAnchorPoint(TargetAnchor, FVector::UpVector);

		bool bReleasedFired = false;
		FVector ExitVel = FVector::ZeroVector;
		GrappleComp->OnGrappleReleased.AddLambda([&bReleasedFired, &ExitVel](const FVector& Vel)
		{
			bReleasedFired = true;
			ExitVel = Vel;
		});

		bool bReleased = GrappleComp->ReleaseAnchorPoint(true);

		TestTrue("Release succeeded", bReleased);
		TestFalse("Not attached after release", GrappleComp->IsAttached());
		TestEqual("State is None", (int32)GrappleComp->GetGrappleState(), (int32)ESBGrappleState::None);
		TestFalse("Grappling tag removed", StateComp->HasTag(Tags.State_Movement_Grappling));
		TestFalse("Swinging tag removed", StateComp->HasTag(Tags.State_Movement_Grappling_Swinging));
		TestFalse("Pulling tag removed", StateComp->HasTag(Tags.State_Movement_Grappling_Pulling));
		TestTrue("Release delegate fired", bReleasedFired);
		TestTrue("Exit velocity has non-zero magnitude", ExitVel.Size() > 0.0f);
	});
}
