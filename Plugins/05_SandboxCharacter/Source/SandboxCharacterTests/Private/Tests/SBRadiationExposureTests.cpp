#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBRadiationExposureComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "SBCharacterTestTypes.h"

BEGIN_DEFINE_SPEC(FSBRadiationExposureTestsSpec, "Sandbox.Character.RadiationExposure", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBRadiationExposureComponent* RadComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBRadiationExposureTestsSpec)

void FSBRadiationExposureTestsSpec::Define()
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

		RadComp = NewObject<USBRadiationExposureComponent>(TestActor, TEXT("RadComp"));
		TestActor->AddOwnedComponent(RadComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(RadComp);
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

	It("Should have zero dose, stage None, and no radiation tags in clean environment", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		RadComp->SetupRadiationComponent(0.0f, 0.0f);

		FSBRadiationEnvironmentData CleanEnv;
		CleanEnv.AmbientDoseRate_mSv_h = 0.0f;

		RadComp->SimulateRadiationTick(5.0f, CleanEnv);

		TestEqual("Accumulated dose is 0", RadComp->GetRadiationData().AccumulatedDose_mSv, 0.0f);
		TestEqual("Stage is None", (int32)RadComp->GetSicknessStage(), (int32)ESBRadiationSicknessStage::None);
		TestFalse("Geiger is not clicking", RadComp->IsGeigerClicking());
		TestFalse("No Exposed tag", StateComp->HasTag(Tags.State_Radiation_Exposed));
		TestFalse("No GeigerClicking tag", StateComp->HasTag(Tags.State_Radiation_GeigerClicking));
	});

	It("Should accumulate dose in radioactive hotspot, trigger Geiger clicking, and enter AcuteRadiationSickness with tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		RadComp->SetupRadiationComponent(0.0f, 0.0f);

		FSBRadiationEnvironmentData HotspotEnv;
		HotspotEnv.AmbientDoseRate_mSv_h = 7200000.0f; // 2000 mSv/s

		USBCharacterTestListener* ARSListener = NewObject<USBCharacterTestListener>(RadComp);
		RadComp->OnAcuteRadiationSicknessTriggered.AddDynamic(ARSListener, &USBCharacterTestListener::OnVoid);

		RadComp->SimulateRadiationTick(1.0f, HotspotEnv);

		TestEqual("Stage is AcuteRadiationSickness", (int32)RadComp->GetSicknessStage(), (int32)ESBRadiationSicknessStage::AcuteRadiationSickness);
		TestTrue("ARS delegate fired", ARSListener->bFired);
		TestTrue("Geiger is clicking", RadComp->IsGeigerClicking());
		TestTrue("Has GeigerClicking tag", StateComp->HasTag(Tags.State_Radiation_GeigerClicking));
		TestTrue("Has AcuteSickness tag", StateComp->HasTag(Tags.State_Radiation_AcuteSickness));
		TestTrue("Has Exposed tag", StateComp->HasTag(Tags.State_Radiation_Exposed));
	});

	It("Should attenuate radiation rate by 80% when equipping lead shielding and grant LeadShielded tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		RadComp->SetupRadiationComponent(0.0f, 0.0f);
		RadComp->EquipLeadShielding(0.8f);

		TestTrue("Has LeadShielded tag", StateComp->HasTag(Tags.State_Radiation_LeadShielded));

		FSBRadiationEnvironmentData HotspotEnv;
		HotspotEnv.AmbientDoseRate_mSv_h = 3600000.0f; // 1000 mSv/s -> 200 mSv/s with 80% shield

		RadComp->SimulateRadiationTick(1.0f, HotspotEnv);

		TestNearlyEqual("Accumulated dose is attenuated (200 mSv)", RadComp->GetRadiationData().AccumulatedDose_mSv, 200.0f, 1.0f);
		TestEqual("Stage remains None under 500 mSv", (int32)RadComp->GetSicknessStage(), (int32)ESBRadiationSicknessStage::None);
	});

	It("Should reduce absorbed dose with antirad medication and decontamination, clearing ARS stage and tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		RadComp->SetupRadiationComponent(2500.0f, 0.0f);

		FSBRadiationEnvironmentData CleanEnv;
		RadComp->SimulateRadiationTick(0.1f, CleanEnv);

		TestEqual("Initially in AcuteRadiationSickness", (int32)RadComp->GetSicknessStage(), (int32)ESBRadiationSicknessStage::AcuteRadiationSickness);
		TestTrue("Has AcuteSickness tag", StateComp->HasTag(Tags.State_Radiation_AcuteSickness));

		RadComp->AdministerAntiradMedication(2200.0f);

		TestEqual("Stage cleared to None", (int32)RadComp->GetSicknessStage(), (int32)ESBRadiationSicknessStage::None);
		TestFalse("AcuteSickness tag cleared", StateComp->HasTag(Tags.State_Radiation_AcuteSickness));
		TestFalse("Exposed tag cleared", StateComp->HasTag(Tags.State_Radiation_Exposed));
	});
}
