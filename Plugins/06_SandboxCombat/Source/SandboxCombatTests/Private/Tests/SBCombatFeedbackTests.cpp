#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBCombatFeedbackComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBCombatFeedbackTestsSpec, "Sandbox.Combat.FeedbackAndHitStop", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* AttackerActor;
	AActor* TargetActor;
	USBCombatFeedbackComponent* FeedbackComp;
	USBStateComponent* AttackerState;
	USBStateComponent* TargetState;
END_DEFINE_SPEC(FSBCombatFeedbackTestsSpec)

void FSBCombatFeedbackTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AttackerActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* AttackerRoot = NewObject<USceneComponent>(AttackerActor, TEXT("AttackerRoot"));
		AttackerActor->SetRootComponent(AttackerRoot);
		AttackerRoot->RegisterComponent();

		AttackerState = NewObject<USBStateComponent>(AttackerActor, TEXT("AttackerState"));
		AttackerState->RegisterComponent();
		AttackerActor->AddOwnedComponent(AttackerState);

		FeedbackComp = NewObject<USBCombatFeedbackComponent>(AttackerActor, TEXT("FeedbackComp"));
		FeedbackComp->RegisterComponent();
		AttackerActor->AddOwnedComponent(FeedbackComp);

		TargetActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* TargetRoot = NewObject<USceneComponent>(TargetActor, TEXT("TargetRoot"));
		TargetActor->SetRootComponent(TargetRoot);
		TargetRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		TargetActor->SetActorLocation(FVector(100.0f, 0.0f, 0.0f));

		TargetState = NewObject<USBStateComponent>(TargetActor, TEXT("TargetState"));
		TargetState->RegisterComponent();
		TargetActor->AddOwnedComponent(TargetState);

		ISBComponentInterface::Execute_OnInitialize(AttackerState);
		ISBComponentInterface::Execute_OnInitialize(FeedbackComp);
		ISBComponentInterface::Execute_OnInitialize(TargetState);
	});

	AfterEach([this]()
	{
		if (AttackerActor)
		{
			AttackerActor->Destroy();
			AttackerActor = nullptr;
		}

		if (TargetActor)
		{
			TargetActor->Destroy();
			TargetActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should apply hit-stop to attacker and target with time dilation and state tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bHitStopCalled = false;
		FeedbackComp->OnHitStopTriggered.AddLambda([&bHitStopCalled](float Duration)
		{
			bHitStopCalled = true;
		});

		FeedbackComp->ApplyHitStop(TargetActor, 0.1f, 0.05f);

		TestTrue("Hit-stop is active", FeedbackComp->IsHitStopActive());
		TestTrue("Attacker has HitStop tag", AttackerState->HasTag(Tags.State_Combat_HitStop));
		TestTrue("Target has HitStop tag", TargetState->HasTag(Tags.State_Combat_HitStop));
		TestNearlyEqual("Attacker dilation is 0.05", AttackerActor->CustomTimeDilation, 0.05f, 0.001f);
		TestNearlyEqual("Target dilation is 0.05", TargetActor->CustomTimeDilation, 0.05f, 0.001f);
		TestTrue("Delegate fired", bHitStopCalled);
	});

	It("Should restore normal time dilation and clear hit-stop tags when duration expires", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FeedbackComp->ApplyHitStop(TargetActor, 0.1f, 0.05f);
		TestTrue("Active before tick", FeedbackComp->IsHitStopActive());

		// Simula 0.15s
		FeedbackComp->TickComponent(0.15f, ELevelTick::LEVELTICK_All, nullptr);

		TestFalse("Hit-stop inactive after duration", FeedbackComp->IsHitStopActive());
		TestNearlyEqual("Attacker dilation restored to 1.0", AttackerActor->CustomTimeDilation, 1.0f, 0.001f);
		TestNearlyEqual("Target dilation restored to 1.0", TargetActor->CustomTimeDilation, 1.0f, 0.001f);
		TestFalse("Attacker HitStop tag removed", AttackerState->HasTag(Tags.State_Combat_HitStop));
		TestFalse("Target HitStop tag removed", TargetState->HasTag(Tags.State_Combat_HitStop));
	});

	It("Should trigger local slomo and state tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bSlomoCalled = false;
		FeedbackComp->OnSlomoTriggered.AddLambda([&bSlomoCalled](float Dilation, float Duration)
		{
			bSlomoCalled = true;
		});

		FeedbackComp->TriggerSlomo(0.2f, 0.5f, false);

		TestTrue("Slomo is active", FeedbackComp->IsSlomoActive());
		TestTrue("Attacker has Slomo tag", AttackerState->HasTag(Tags.State_Combat_Slomo));
		TestNearlyEqual("Attacker dilation is 0.2", AttackerActor->CustomTimeDilation, 0.2f, 0.001f);
		TestTrue("Slomo delegate fired", bSlomoCalled);

		// Simula 0.6s
		FeedbackComp->TickComponent(0.6f, ELevelTick::LEVELTICK_All, nullptr);

		TestFalse("Slomo inactive after duration", FeedbackComp->IsSlomoActive());
		TestNearlyEqual("Attacker dilation restored", AttackerActor->CustomTimeDilation, 1.0f, 0.001f);
		TestFalse("Slomo tag removed", AttackerState->HasTag(Tags.State_Combat_Slomo));
	});

	It("Should execute combined combat feedback profile", [this]()
	{
		FSBCombatFeedbackProfile Profile;
		Profile.HitStop.Duration = 0.08f;
		Profile.HitStop.TimeDilation = 0.02f;
		Profile.Slomo.Duration = 0.4f;
		Profile.Slomo.TargetDilation = 0.3f;

		FeedbackComp->ApplyCombatFeedback(TargetActor, Profile);

		TestTrue("Hit-stop active from profile", FeedbackComp->IsHitStopActive());
		TestTrue("Slomo active from profile", FeedbackComp->IsSlomoActive());
	});
}
