#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBGliderComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBGliderTestsSpec, "Sandbox.Character.GlidingAndAerialLocomotion", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBGliderComponent* GliderComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBGliderTestsSpec)

void FSBGliderTestsSpec::Define()
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

		GliderComp = NewObject<USBGliderComponent>(CharacterActor, TEXT("GliderComp"));
		CharacterActor->AddOwnedComponent(GliderComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(GliderComp);
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

	It("Should deploy glider in air, set gliding state, and apply Gliding state tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bStateChangedFired = false;
		ESBGliderState FiredState = ESBGliderState::Retracted;
		GliderComp->OnGliderStateChanged.AddLambda([&bStateChangedFired, &FiredState](ESBGliderState NewState)
		{
			bStateChangedFired = true;
			FiredState = NewState;
		});

		bool bDeployed = GliderComp->DeployGlider();

		TestTrue("Deploy succeeded", bDeployed);
		TestTrue("Is gliding", GliderComp->IsGliding());
		TestEqual("State is Gliding", (int32)GliderComp->GetGliderState(), (int32)ESBGliderState::Gliding);
		TestEqual("Glide fall speed matches settings", GliderComp->GetFlightData().CurrentFallSpeed, 200.0f);
		TestEqual("Glide forward speed matches settings", GliderComp->GetFlightData().CurrentForwardSpeed, 800.0f);
		TestTrue("Has Gliding tag", StateComp->HasTag(Tags.State_Movement_Gliding));
		TestFalse("Does not have Diving tag", StateComp->HasTag(Tags.State_Movement_Gliding_Diving));
		TestTrue("Delegate fired", bStateChangedFired);
		TestEqual("Delegate state is Gliding", (int32)FiredState, (int32)ESBGliderState::Gliding);
	});

	It("Should transition to aerial dive and grant Diving tag with increased speeds", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		GliderComp->DeployGlider();

		bool bDiveStarted = GliderComp->StartAerialDive();

		TestTrue("Start dive succeeded", bDiveStarted);
		TestTrue("Is diving", GliderComp->IsDiving());
		TestEqual("State is Diving", (int32)GliderComp->GetGliderState(), (int32)ESBGliderState::Diving);
		TestEqual("Dive fall speed matches settings", GliderComp->GetFlightData().CurrentFallSpeed, 900.0f);
		TestEqual("Dive forward speed matches settings", GliderComp->GetFlightData().CurrentForwardSpeed, 1200.0f);
		TestTrue("Has Gliding tag", StateComp->HasTag(Tags.State_Movement_Gliding));
		TestTrue("Has Diving tag", StateComp->HasTag(Tags.State_Movement_Gliding_Diving));
	});

	It("Should stop aerial dive and restore gliding speeds", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		GliderComp->DeployGlider();
		GliderComp->StartAerialDive();

		bool bDiveStopped = GliderComp->StopAerialDive();

		TestTrue("Stop dive succeeded", bDiveStopped);
		TestFalse("No longer diving", GliderComp->IsDiving());
		TestEqual("State is Gliding", (int32)GliderComp->GetGliderState(), (int32)ESBGliderState::Gliding);
		TestEqual("Fall speed restored", GliderComp->GetFlightData().CurrentFallSpeed, 200.0f);
		TestEqual("Forward speed restored", GliderComp->GetFlightData().CurrentForwardSpeed, 800.0f);
		TestTrue("Has Gliding tag", StateComp->HasTag(Tags.State_Movement_Gliding));
		TestFalse("No longer has Diving tag", StateComp->HasTag(Tags.State_Movement_Gliding_Diving));
	});

	It("Should retract glider and clear all aerial state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		GliderComp->DeployGlider();
		GliderComp->StartAerialDive();

		TestTrue("Gliding before retract", GliderComp->IsGliding());

		bool bRetracted = GliderComp->RetractGlider();

		TestTrue("Retract succeeded", bRetracted);
		TestFalse("Not gliding after retract", GliderComp->IsGliding());
		TestEqual("State is Retracted", (int32)GliderComp->GetGliderState(), (int32)ESBGliderState::Retracted);
		TestEqual("Fall speed is zero", GliderComp->GetFlightData().CurrentFallSpeed, 0.0f);
		TestFalse("Gliding tag removed", StateComp->HasTag(Tags.State_Movement_Gliding));
		TestFalse("Diving tag removed", StateComp->HasTag(Tags.State_Movement_Gliding_Diving));
	});
}
