#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBExecutionComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBExecutionTestsSpec, "Sandbox.Combat.Executions", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* AttackerActor;
	AActor* VictimActor;
	USBExecutionComponent* ExecComp;
	USBStateComponent* AttackerState;
	USBStateComponent* VictimState;
	USBAttributeComponent* VictimAttributes;
END_DEFINE_SPEC(FSBExecutionTestsSpec)

void FSBExecutionTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 1. Atacante em (0, 0, 0)
		AttackerActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* AttackerRoot = NewObject<USceneComponent>(AttackerActor, TEXT("AttackerRoot"));
		AttackerActor->SetRootComponent(AttackerRoot);
		AttackerRoot->RegisterComponent();

		AttackerState = NewObject<USBStateComponent>(AttackerActor, TEXT("AttackerState"));
		AttackerState->RegisterComponent();
		AttackerActor->AddOwnedComponent(AttackerState);

		ExecComp = NewObject<USBExecutionComponent>(AttackerActor, TEXT("ExecComp"));
		ExecComp->RegisterComponent();
		AttackerActor->AddOwnedComponent(ExecComp);

		// 2. Vítima em (500, 500, 0)
		VictimActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(500.0f, 500.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		USceneComponent* VictimRoot = NewObject<USceneComponent>(VictimActor, TEXT("VictimRoot"));
		VictimActor->SetRootComponent(VictimRoot);
		VictimRoot->RegisterComponent();
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		VictimActor->SetActorLocation(FVector(500.0f, 500.0f, 0.0f));

		VictimState = NewObject<USBStateComponent>(VictimActor, TEXT("VictimState"));
		VictimState->RegisterComponent();
		VictimActor->AddOwnedComponent(VictimState);

		VictimAttributes = NewObject<USBAttributeComponent>(VictimActor, TEXT("VictimAttributes"));
		VictimAttributes->RegisterComponent();
		VictimActor->AddOwnedComponent(VictimAttributes);

		ISBComponentInterface::Execute_OnInitialize(AttackerState);
		ISBComponentInterface::Execute_OnInitialize(ExecComp);
		ISBComponentInterface::Execute_OnInitialize(VictimState);
		ISBComponentInterface::Execute_OnInitialize(VictimAttributes);

		// Configura vida base da vítima
		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.0f;
		HealthAttr.CurrentValue = 100.0f;
		HealthAttr.MinValue = 0.0f;
		HealthAttr.MaxValue = 100.0f;
		VictimAttributes->RegisterAttribute(FSBGameplayTags::Get().Attribute_Health, HealthAttr);
	});

	AfterEach([this]()
	{
		if (AttackerActor)
		{
			AttackerActor->Destroy();
			AttackerActor = nullptr;
		}

		if (VictimActor)
		{
			VictimActor->Destroy();
			VictimActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should initialize execution, align victim, and apply combat state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBExecutionPairDefinition Def;
		Def.ExecutionId = FName("Finisher_ThroatCut");
		Def.RelativeVictimLocation = FVector(120.0f, 0.0f, 0.0f);
		Def.RelativeVictimRotation = FRotator(0.0f, 180.0f, 0.0f);
		Def.ExecutionDuration = 2.0f;
		Def.bGrantInvulnerabilityToAttacker = true;

		bool bStarted = ExecComp->StartExecution(VictimActor, Def);
		TestTrue("Execution started successfully", bStarted);
		TestTrue("ExecComp is executing", ExecComp->IsExecuting());

		// Verifica alinhamento
		TestNearlyEqual("Victim aligned to X=120", (float)VictimActor->GetActorLocation().X, 120.0f, 1.0f);
		TestNearlyEqual("Victim aligned to Y=0", (float)VictimActor->GetActorLocation().Y, 0.0f, 1.0f);

		// Verifica tags
		TestTrue("Attacker has Executing tag", AttackerState->HasTag(Tags.State_Combat_Executing));
		TestTrue("Attacker has Invulnerable tag", AttackerState->HasTag(Tags.State_Combat_Invulnerable));
		TestTrue("Victim has Executed tag", VictimState->HasTag(Tags.State_Combat_Executed));
	});

	It("Should complete execution sequence, apply damage, and trigger OnExecutionFinished", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBExecutionPairDefinition Def;
		Def.ExecutionId = FName("Finisher_Decapitation");
		Def.ExecutionDuration = 1.0f;
		Def.DamageOnFinish = 100.0f;

		ExecComp->StartExecution(VictimActor, Def);

		// Simula 1.2s de tick
		ExecComp->TickComponent(1.2f, ELevelTick::LEVELTICK_All, nullptr);

		TestFalse("No longer executing", ExecComp->IsExecuting());
		TestEqual("State is Finished", (int32)ExecComp->GetExecutionState(), (int32)ESBExecutionState::Finished);
		TestEqual("Victim health reduced to 0", VictimAttributes->GetAttributeValue(Tags.Attribute_Health), 0.0f);
		TestFalse("Attacker Executing tag removed", AttackerState->HasTag(Tags.State_Combat_Executing));
		TestFalse("Victim Executed tag removed", VictimState->HasTag(Tags.State_Combat_Executed));
	});

	It("Should block starting another execution when already executing", [this]()
	{
		FSBExecutionPairDefinition Def;
		Def.ExecutionDuration = 2.0f;

		ExecComp->StartExecution(VictimActor, Def);
		TestTrue("First execution active", ExecComp->IsExecuting());

		AActor* SecondVictim = TestWorld->SpawnActor<AActor>();
		bool bSecondStarted = ExecComp->StartExecution(SecondVictim, Def);
		TestFalse("Second execution rejected", bSecondStarted);

		SecondVictim->Destroy();
	});

	It("Should abort execution and broadcast OnExecutionAborted", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBExecutionPairDefinition Def;
		Def.ExecutionDuration = 2.0f;

		ExecComp->StartExecution(VictimActor, Def);
		TestTrue("Executing before abort", ExecComp->IsExecuting());

		ExecComp->StopExecution(true);

		TestFalse("No longer executing after abort", ExecComp->IsExecuting());
		TestEqual("State is Aborted", (int32)ExecComp->GetExecutionState(), (int32)ESBExecutionState::Aborted);
		TestFalse("Attacker Executing tag removed", AttackerState->HasTag(Tags.State_Combat_Executing));
		TestFalse("Victim Executed tag removed", VictimState->HasTag(Tags.State_Combat_Executed));
	});
}
