#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Subsystems/SBWorldIntegritySubsystem.h"
#include "Components/SBWorldIntegrityAuditorComponent.h"
#include "SBCoreTestTypes.h"
#include "SBGameplayTags.h"
#include "GameFramework/Actor.h"

BEGIN_DEFINE_SPEC(FSBWorldIntegrityTestsSpec, "Sandbox.Core.WorldIntegrity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
	UWorld* TestWorld = nullptr;
	USBWorldIntegritySubsystem* IntegritySubsystem = nullptr;
	AActor* TestActor = nullptr;
	USBCoreTestStateComponent* StateComp = nullptr;
	USBWorldIntegrityAuditorComponent* AuditorComp = nullptr;
END_DEFINE_SPEC(FSBWorldIntegrityTestsSpec)

void FSBWorldIntegrityTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("IntegrityTestWorld"));
		if (TestWorld)
		{
			IntegritySubsystem = TestWorld->GetSubsystem<USBWorldIntegritySubsystem>();
			if (IntegritySubsystem)
			{
				IntegritySubsystem->ResetSubsystem();
			}

			TestActor = TestWorld->SpawnActor<AActor>();
			if (TestActor)
			{
				StateComp = NewObject<USBCoreTestStateComponent>(TestActor, TEXT("StateComp"));
				TestActor->AddInstanceComponent(StateComp);
				StateComp->RegisterComponent();

				AuditorComp = NewObject<USBWorldIntegrityAuditorComponent>(TestActor, TEXT("AuditorComp"));
				TestActor->AddInstanceComponent(AuditorComp);
				AuditorComp->RegisterComponent();
			}
		}
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
			IntegritySubsystem = nullptr;
			TestActor = nullptr;
			StateComp = nullptr;
			AuditorComp = nullptr;
		}
	});

	It("Should audit recipe and loot table integrity, capturing validation failures", [this]()
	{
		TestNotNull("IntegritySubsystem valid", IntegritySubsystem);

		TestTrue("Valid recipe with ingredients passes", IntegritySubsystem->AuditRecipe(TEXT("Recipe_IronIngot"), 2, 1));
		TestFalse("Invalid recipe with 0 ingredients fails", IntegritySubsystem->AuditRecipe(TEXT("Recipe_Broken"), 0, 1));

		TestTrue("Valid loot table passes", IntegritySubsystem->AuditLootTable(TEXT("Loot_CommonChest"), 100.0f));
		TestFalse("Invalid loot table with 0 weight fails", IntegritySubsystem->AuditLootTable(TEXT("Loot_Empty"), 0.0f));

		FSBWorldIntegrityReport Report = IntegritySubsystem->GenerateReport();
		TestEqual("Total issues found is 2", Report.TotalIssuesFound, 2);
		// CriticalErrorsCount agrega Error + Critical: sao as duas severidades bloqueantes, e
		// o relatorio nao tem bucket separado para Error. Contar apenas Critical deixaria a
		// receita invalida fora de qualquer contador (2 problemas, 1 contabilizado).
		// Aqui: receita quebrada (Error) + loot table vazia (Critical) = 2.
		TestEqual("Blocking issues is 2 (recipe Error + loot Critical)", Report.CriticalErrorsCount, 2);
		TestFalse("Report failed due to critical errors", Report.bPassed);
	});

	It("Should audit network connections and register issues with severity levels", [this]()
	{
		TestNotNull("IntegritySubsystem valid", IntegritySubsystem);

		TestTrue("Powered grid passes", IntegritySubsystem->AuditNetworkConnection(TEXT("Node_Substation"), true, true));
		TestFalse("Consumer without power fails audit", IntegritySubsystem->AuditNetworkConnection(TEXT("Node_OrphanConsumer"), false, true));

		FSBWorldIntegrityReport Report = IntegritySubsystem->GenerateReport();
		TestEqual("Total issues found is 1", Report.TotalIssuesFound, 1);
		TestEqual("Warnings count is 1", Report.WarningsCount, 1);
		TestTrue("Report passed with only warnings", Report.bPassed);
	});

	It("Should auto-fix fixable issues and generate passing report", [this]()
	{
		TestNotNull("IntegritySubsystem valid", IntegritySubsystem);

		IntegritySubsystem->AuditLootTable(TEXT("Loot_AutoFixable"), 0.0f); // Critical, autoFixable=true

		FSBWorldIntegrityReport InitialReport = IntegritySubsystem->GenerateReport();
		TestFalse("Initial report failed", InitialReport.bPassed);

		int32 FixedCount = IntegritySubsystem->AutoFixIssues();
		TestEqual("Auto fixed 1 issue", FixedCount, 1);

		FSBWorldIntegrityReport FinalReport = IntegritySubsystem->GenerateReport();
		TestTrue("Final report passed after auto fix", FinalReport.bPassed);
		TestEqual("Final report issues is 0", FinalReport.TotalIssuesFound, 0);
		TestEqual("AutoFixedCount recorded in report", FinalReport.AutoFixedCount, 1);
	});

	It("Should auto-subscribe integrity auditor component, detect issues, update state tags, and cleanup", [this]()
	{
		TestNotNull("AuditorComp valid", AuditorComp);
		TestNotNull("StateComp valid", StateComp);

		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AuditorComp->OnInitialize_Implementation();
		TestTrue("Audited tag present", StateComp->HasTag(Tags.State_Integrity_Audited));
		TestTrue("Clean tag present initially", StateComp->HasTag(Tags.State_Integrity_Clean));
		TestFalse("IssueDetected tag not present initially", StateComp->HasTag(Tags.State_Integrity_IssueDetected));

		// Injeta issue crítico
		AuditorComp->InjectTestIssue(TEXT("CorruptData"), TEXT("Corrupt structure transform"), ESBIntegritySeverity::Critical, false);
		TestFalse("Clean tag removed after issue", StateComp->HasTag(Tags.State_Integrity_Clean));
		TestTrue("IssueDetected tag granted after issue", StateComp->HasTag(Tags.State_Integrity_IssueDetected));

		// Shutdown
		AuditorComp->OnShutdown_Implementation();
		TestFalse("Audited tag removed on shutdown", StateComp->HasTag(Tags.State_Integrity_Audited));
		TestFalse("Clean tag removed on shutdown", StateComp->HasTag(Tags.State_Integrity_Clean));
		TestFalse("IssueDetected tag removed on shutdown", StateComp->HasTag(Tags.State_Integrity_IssueDetected));
	});
}
