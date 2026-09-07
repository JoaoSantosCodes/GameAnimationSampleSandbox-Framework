#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBAfflictionComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "SBCharacterTestTypes.h"

BEGIN_DEFINE_SPEC(FSBAfflictionTestsSpec, "Sandbox.Character.Afflictions", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBAfflictionComponent* AfflictionComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBAfflictionTestsSpec)

void FSBAfflictionTestsSpec::Define()
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

		StateComp = NewObject<USBStateComponent>(TestActor, TEXT("StateComp"));
		TestActor->AddOwnedComponent(StateComp);

		AfflictionComp = NewObject<USBAfflictionComponent>(TestActor, TEXT("AfflictionComp"));
		TestActor->AddOwnedComponent(AfflictionComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(AfflictionComp);
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

	It("Should have zero afflictions, full motor speed multiplier, and no affliction tags initially", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AfflictionComp->SetupAfflictionComponent(0.0f);

		TestEqual("No active afflictions", AfflictionComp->GetAfflictionState().ActiveAfflictions.Num(), 0);
		TestEqual("Motor multiplier is 1.0", AfflictionComp->GetMotorSpeedMultiplier(), 1.0f);
		TestFalse("Not motor impaired", AfflictionComp->IsMotorImpaired());
		TestFalse("No Impaired tag", StateComp->HasTag(Tags.State_Affliction_Impaired));
		TestFalse("No Paralyzed tag", StateComp->HasTag(Tags.State_Affliction_Paralyzed));
	});

	It("Should apply severe motor impairment affliction, reduce speed multiplier, and grant Impaired and Paralyzed tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AfflictionComp->SetupAfflictionComponent(0.0f);
		AfflictionComp->ApplyAffliction(ESBAfflictionType::MotorImpairment, 0.85f, 10.0f);

		TestEqual("One active affliction", AfflictionComp->GetAfflictionState().ActiveAfflictions.Num(), 1);
		TestNearlyEqual("Motor multiplier is 0.15", AfflictionComp->GetMotorSpeedMultiplier(), 0.15f, 0.01f);
		TestTrue("Is motor impaired", AfflictionComp->IsMotorImpaired());
		TestTrue("Has Impaired tag", StateComp->HasTag(Tags.State_Affliction_Impaired));
		TestTrue("Has Paralyzed tag", StateComp->HasTag(Tags.State_Affliction_Paralyzed));
	});

	It("Should cure motor impairment with synthesized antidote, restoring full speed and removing tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AfflictionComp->SetupAfflictionComponent(0.0f);
		AfflictionComp->ApplyAffliction(ESBAfflictionType::MotorImpairment, 0.85f, 10.0f);

		TestTrue("Initially has Impaired tag", StateComp->HasTag(Tags.State_Affliction_Impaired));

		USBCharacterTestListener* AfflictionListener = NewObject<USBCharacterTestListener>(AfflictionComp);
		AfflictionComp->OnAfflictionNeutralized.AddDynamic(AfflictionListener, &USBCharacterTestListener::OnAffliction);

		AfflictionComp->ApplyNeutralizer(ESBNeutralizerType::SynthesizedAntidote, 1.0f);

		TestEqual("Afflictions cleared", AfflictionComp->GetAfflictionState().ActiveAfflictions.Num(), 0);
		TestTrue("Neutralized delegate fired for MotorImpairment", AfflictionListener->bFired && AfflictionListener->AfflictionArg == ESBAfflictionType::MotorImpairment);
		TestEqual("Speed restored to 1.0", AfflictionComp->GetMotorSpeedMultiplier(), 1.0f);
		TestFalse("No longer impaired", AfflictionComp->IsMotorImpaired());
		TestFalse("Impaired tag removed", StateComp->HasTag(Tags.State_Affliction_Impaired));
		TestFalse("Paralyzed tag removed", StateComp->HasTag(Tags.State_Affliction_Paralyzed));
	});

	It("Should grant Inoculated tag and attenuate incoming affliction severity by inoculation resistance", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AfflictionComp->SetupAfflictionComponent(0.0f);
		AfflictionComp->ApplyInoculation(0.5f, 60.0f);

		TestTrue("Has Inoculated tag", StateComp->HasTag(Tags.State_Affliction_Inoculated));

		AfflictionComp->ApplyAffliction(ESBAfflictionType::TissueDegradation, 0.6f, 10.0f);

		TestEqual("One active affliction", AfflictionComp->GetAfflictionState().ActiveAfflictions.Num(), 1);
		TestNearlyEqual("Severity attenuated by 50% (0.3)", AfflictionComp->GetAfflictionState().ActiveAfflictions[0].Severity, 0.3f, 0.01f);
		TestTrue("Has Degradation tag", StateComp->HasTag(Tags.State_Affliction_Degradation));
	});
}
