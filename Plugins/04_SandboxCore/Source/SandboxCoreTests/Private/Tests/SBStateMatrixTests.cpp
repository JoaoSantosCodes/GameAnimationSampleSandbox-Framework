#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Subsystems/SBStateMatrixSubsystem.h"
#include "Components/SBStateMatrixGuardComponent.h"
#include "SBCoreTestTypes.h"
#include "SBGameplayTags.h"
#include "GameFramework/Actor.h"

BEGIN_DEFINE_SPEC(FSBStateMatrixTestsSpec, "Sandbox.Core.StateMatrix", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	UWorld* TestWorld = nullptr;
	USBStateMatrixSubsystem* MatrixSubsystem = nullptr;
	AActor* TestActor = nullptr;
	USBCoreTestStateComponent* StateComp = nullptr;
	USBStateMatrixGuardComponent* GuardComp = nullptr;
END_DEFINE_SPEC(FSBStateMatrixTestsSpec)

void FSBStateMatrixTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("StateMatrixTestWorld"));
		if (TestWorld)
		{
			MatrixSubsystem = TestWorld->GetSubsystem<USBStateMatrixSubsystem>();
			if (MatrixSubsystem)
			{
				MatrixSubsystem->ResetSubsystem();
				MatrixSubsystem->RegisterStandardRules();
			}

			TestActor = TestWorld->SpawnActor<AActor>();
			if (TestActor)
			{
				StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
				TestActor->AddInstanceComponent(StateComp);
				StateComp->RegisterComponent();

				GuardComp = NewObject<USBStateMatrixGuardComponent>(TestActor, TEXT("GuardComp"));
				TestActor->AddInstanceComponent(GuardComp);
				GuardComp->RegisterComponent();
			}
		}
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
			MatrixSubsystem = nullptr;
			TestActor = nullptr;
			StateComp = nullptr;
			GuardComp = nullptr;
		}
	});

	It("Should register custom rule and allow non-conflicting tag addition", [this]()
	{
		TestNotNull("MatrixSubsystem valid", MatrixSubsystem);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBStateMatrixMetrics InitialMetrics = MatrixSubsystem->GetMetrics();
		TestTrue("Standard rules registered (>=3)", InitialMetrics.TotalRulesRegistered >= 3);

		FGameplayTagContainer CurrentTags;
		FGameplayTagContainer PrunedTags;
		bool bAllowed = MatrixSubsystem->ValidateTagAddition(TestActor, Tags.State_Combat_Attacking, CurrentTags, PrunedTags);

		TestTrue("Non-conflicting tag addition allowed", bAllowed);
		TestEqual("Pruned tags count is 0", PrunedTags.Num(), 0);

		FSBStateMatrixMetrics AfterMetrics = MatrixSubsystem->GetMetrics();
		TestEqual("Total evaluations is 1", AfterMetrics.TotalEvaluations, 1);
		TestEqual("Total violations is 0", AfterMetrics.TotalViolationsDetected, 0);
	});

	It("Should reject invalid tag addition when conflicting with existing tag under RejectTransition rule", [this]()
	{
		TestNotNull("MatrixSubsystem valid", MatrixSubsystem);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FGameplayTagContainer CurrentTags;
		CurrentTags.AddTag(Tags.State_Combat_Executing);

		FGameplayTagContainer PrunedTags;
		// Standard rule: State_Character_Dead rejects if State_Combat_Executing is present
		bool bAllowed = MatrixSubsystem->ValidateTagAddition(TestActor, Tags.State_Character_Dead, CurrentTags, PrunedTags);

		TestFalse("Conflicting tag addition rejected", bAllowed);

		FSBStateMatrixMetrics Metrics = MatrixSubsystem->GetMetrics();
		TestEqual("Violations detected is 1", Metrics.TotalViolationsDetected, 1);
		TestEqual("Transitions blocked is 1", Metrics.TotalTransitionsBlocked, 1);
	});

	It("Should prune incompatible tags automatically under AutoResolvePrune rule and allow transition", [this]()
	{
		TestNotNull("MatrixSubsystem valid", MatrixSubsystem);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FGameplayTagContainer CurrentTags;
		CurrentTags.AddTag(Tags.State_Movement_Swimming);
		CurrentTags.AddTag(Tags.State_Movement_Swimming_Diving);

		FGameplayTagContainer PrunedTags;
		// Standard rule: State_Movement_Gliding autorsolves and prunes Swimming/Diving
		bool bAllowed = MatrixSubsystem->ValidateTagAddition(TestActor, Tags.State_Movement_Gliding, CurrentTags, PrunedTags);

		TestTrue("Transition allowed with auto-prune", bAllowed);
		TestTrue("Swimming tag pruned", PrunedTags.HasTag(Tags.State_Movement_Swimming));
		TestTrue("Diving tag pruned", PrunedTags.HasTag(Tags.State_Movement_Swimming_Diving));

		FSBStateMatrixMetrics Metrics = MatrixSubsystem->GetMetrics();
		TestEqual("Tags auto-pruned is 2", Metrics.TotalTagsAutoPruned, 2);
	});

	It("Should auto-verify actor with StateMatrixGuardComponent, prune conflicts on apply, and cleanup on shutdown", [this]()
	{
		TestNotNull("GuardComp valid", GuardComp);
		TestNotNull("StateComp valid", StateComp);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		GuardComp->OnInitialize_Implementation();
		TestTrue("State.Matrix.Verified tag granted", StateComp->HasTag(Tags.State_Matrix_Verified));
		TestTrue("State.Matrix.StrictEnforcement tag granted", StateComp->HasTag(Tags.State_Matrix_StrictEnforcement));

		// Define estado inicial como Swimming
		StateComp->AddTag(Tags.State_Movement_Swimming);
		TestTrue("Initial state has Swimming", StateComp->HasTag(Tags.State_Movement_Swimming));

		// Tenta aplicar Gliding -> deve podar Swimming e conceder Gliding
		bool bApplied = GuardComp->TryApplyStateTag(Tags.State_Movement_Gliding);
		TestTrue("Gliding applied successfully", bApplied);
		TestTrue("Gliding is now active", StateComp->HasTag(Tags.State_Movement_Gliding));
		TestFalse("Swimming was pruned", StateComp->HasTag(Tags.State_Movement_Swimming));

		// Shutdown
		GuardComp->OnShutdown_Implementation();
		TestFalse("Matrix.Verified removed on shutdown", StateComp->HasTag(Tags.State_Matrix_Verified));
		TestFalse("Matrix.StrictEnforcement removed on shutdown", StateComp->HasTag(Tags.State_Matrix_StrictEnforcement));
	});
}
