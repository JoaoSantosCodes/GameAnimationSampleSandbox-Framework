#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBFootIKComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBFootIKTestsSpec, "Sandbox.Character.FootIKAndGroundAdaptation", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBFootIKComponent* FootIKComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBFootIKTestsSpec)

void FSBFootIKTestsSpec::Define()
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

		FootIKComp = NewObject<USBFootIKComponent>(CharacterActor, TEXT("FootIKComp"));
		CharacterActor->AddOwnedComponent(FootIKComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(FootIKComp);
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

	It("Should calculate zero offset on flat ground and grant FootIKActive tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bDelegateFired = false;
		FootIKComp->OnFootIKUpdated.AddLambda([&bDelegateFired](const FSBFootIKResult& Res)
		{
			bDelegateFired = true;
		});

		FSBFootIKResult Result = FootIKComp->CalculateFootIK(0.0f, FVector::UpVector, 0.0f, FVector::UpVector);

		TestTrue("Delegate fired", bDelegateFired);
		TestEqual("Left foot offset is 0", Result.LeftFoot.FootOffset, 0.0f);
		TestEqual("Right foot offset is 0", Result.RightFoot.FootOffset, 0.0f);
		TestEqual("Pelvis offset is 0", Result.PelvisOffset, 0.0f);
		TestFalse("Not on slope", Result.bIsOnSlope);
		TestTrue("Has FootIKActive tag", StateComp->HasTag(Tags.State_Movement_FootIKActive));
		TestFalse("Does not have OnSlope tag", StateComp->HasTag(Tags.State_Movement_OnSlope));
	});

	It("Should calculate pelvis offset on uneven surface/stair step", [this]()
	{
		// Left foot on a step (+15cm), Right foot on ground (0cm)
		FSBFootIKResult StepUp = FootIKComp->CalculateFootIK(15.0f, FVector::UpVector, 0.0f, FVector::UpVector);
		TestEqual("Left foot offset is 15", StepUp.LeftFoot.FootOffset, 15.0f);
		TestEqual("Pelvis offset is min (0)", StepUp.PelvisOffset, 0.0f);

		// Left foot in a hole (-12cm), Right foot on step (+8cm)
		FSBFootIKResult StepDown = FootIKComp->CalculateFootIK(-12.0f, FVector::UpVector, 8.0f, FVector::UpVector);
		TestEqual("Pelvis offset compensates downward (-12)", StepDown.PelvisOffset, -12.0f);
	});

	It("Should calculate foot rotation on slope and apply OnSlope state tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// Normal inclined 20 degrees
		FVector SlopedNormal = FVector(0.0f, 0.35f, 0.93f).GetSafeNormal();
		FSBFootIKResult SlopedResult = FootIKComp->CalculateFootIK(5.0f, SlopedNormal, 0.0f, SlopedNormal);

		TestTrue("Detected as slope", SlopedResult.bIsOnSlope);
		TestTrue("Has OnSlope tag", StateComp->HasTag(Tags.State_Movement_OnSlope));
		TestNotEqual("Left foot roll is non-zero", (float)SlopedResult.LeftFoot.FootRotation.Roll, 0.0f);
	});

	It("Should reset foot IK and remove state tags when disabled", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FVector SlopedNormal = FVector(0.0f, 0.35f, 0.93f).GetSafeNormal();
		FootIKComp->CalculateFootIK(5.0f, SlopedNormal, 0.0f, SlopedNormal);

		TestTrue("Has FootIKActive tag before reset", StateComp->HasTag(Tags.State_Movement_FootIKActive));

		FootIKComp->SetIKEnabled(false);

		TestFalse("IK is disabled", FootIKComp->IsIKEnabled());
		TestFalse("FootIKActive tag removed", StateComp->HasTag(Tags.State_Movement_FootIKActive));
		TestFalse("OnSlope tag removed", StateComp->HasTag(Tags.State_Movement_OnSlope));
	});
}
