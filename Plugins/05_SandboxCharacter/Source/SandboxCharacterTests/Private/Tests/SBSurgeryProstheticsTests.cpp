#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBSurgeryProstheticsComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "SBCharacterTestTypes.h"

BEGIN_DEFINE_SPEC(FSBSurgeryProstheticsTestsSpec, "Sandbox.Character.SurgeryProsthetics", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBSurgeryProstheticsComponent* SurgeryComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBSurgeryProstheticsTestsSpec)

void FSBSurgeryProstheticsTestsSpec::Define()
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

		SurgeryComp = NewObject<USBSurgeryProstheticsComponent>(TestActor, TEXT("SurgeryComp"));
		TestActor->AddOwnedComponent(SurgeryComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(SurgeryComp);
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

	It("Should initialize with 100% organ health, no prosthetics, Idle state, and no surgery tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SurgeryComp->SetupSurgicalComponent(100.0f);

		TestEqual("OperationState is Idle", (int32)SurgeryComp->GetPatientData().OperationState, (int32)ESBSurgicalOperationState::Idle);
		TestEqual("No installed prosthetics", SurgeryComp->GetPatientData().InstalledProsthetics.Num(), 0);
		TestFalse("Not anesthetized", SurgeryComp->IsUnderAnesthesia());
		TestFalse("No UnderAnesthesia tag", StateComp->HasTag(Tags.State_Surgery_UnderAnesthesia));
		TestFalse("No Operating tag", StateComp->HasTag(Tags.State_Surgery_Operating));
	});

	It("Should administer anesthesia, start surgical operation, advance progress on tick, and broadcast completion", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SurgeryComp->SetupSurgicalComponent(100.0f);

		USBCharacterTestListener* AnesthesiaListener = NewObject<USBCharacterTestListener>(SurgeryComp);
		SurgeryComp->OnAnesthesiaStateChanged.AddDynamic(AnesthesiaListener, &USBCharacterTestListener::OnBool);

		SurgeryComp->AdministerAnesthesia(30.0f);

		TestTrue("Anesthesia delegate fired with active state", AnesthesiaListener->bFired && AnesthesiaListener->BoolArg);
		TestTrue("Patient is anesthetized", SurgeryComp->IsUnderAnesthesia());
		TestTrue("Has UnderAnesthesia tag", StateComp->HasTag(Tags.State_Surgery_UnderAnesthesia));

		SurgeryComp->StartSurgicalOperation(2.0f);
		TestTrue("Has Operating tag", StateComp->HasTag(Tags.State_Surgery_Operating));

		USBCharacterTestListener* OperationListener = NewObject<USBCharacterTestListener>(SurgeryComp);
		SurgeryComp->OnSurgicalOperationCompleted.AddDynamic(OperationListener, &USBCharacterTestListener::OnVoid);

		SurgeryComp->SimulateSurgicalTick(2.0f);

		TestTrue("Operation completed delegate fired", OperationListener->bFired);
		TestEqual("State is PostOpRecovery", (int32)SurgeryComp->GetPatientData().OperationState, (int32)ESBSurgicalOperationState::PostOpRecovery);
	});

	It("Should install cybernetic prosthetic limb, calculate efficiency bonus, and grant CyberneticAugmented tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SurgeryComp->SetupSurgicalComponent(100.0f);
		SurgeryComp->InstallProsthetic(ESBSurgicalLimbType::RightArm, ESBProstheticGrade::CyberneticAugment, 1.5f);

		TestTrue("Has RightArm prosthetic", SurgeryComp->HasProsthetic(ESBSurgicalLimbType::RightArm));
		TestNearlyEqual("Overall efficiency bonus is 0.5", SurgeryComp->GetOverallEfficiencyBonus(), 0.5f, 0.01f);
		TestTrue("Has ProstheticInstalled tag", StateComp->HasTag(Tags.State_Surgery_ProstheticInstalled));
		TestTrue("Has CyberneticAugmented tag", StateComp->HasTag(Tags.State_Surgery_CyberneticAugmented));
	});

	It("Should warn organ rejection on transplant without immunosuppressant and clear warning after administering medicine", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SurgeryComp->SetupSurgicalComponent(50.0f);

		USBCharacterTestListener* RejectionListener = NewObject<USBCharacterTestListener>(SurgeryComp);
		SurgeryComp->OnOrganRejectionWarning.AddDynamic(RejectionListener, &USBCharacterTestListener::OnVoid);

		SurgeryComp->PerformOrganTransplant(100.0f);

		TestTrue("Organ rejection warning fired", RejectionListener->bFired);
		TestTrue("Is organ rejection risk", SurgeryComp->GetPatientData().bIsOrganRejectionRisk);
		TestTrue("Has OrganRejection tag", StateComp->HasTag(Tags.State_Surgery_OrganRejection));

		SurgeryComp->AdministerImmunosuppressant(0.8f);

		TestFalse("Organ rejection risk cleared", SurgeryComp->GetPatientData().bIsOrganRejectionRisk);
		TestFalse("OrganRejection tag removed", StateComp->HasTag(Tags.State_Surgery_OrganRejection));
	});
}
