// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBAtmosphericSafetyComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "SBCharacterTestTypes.h"

BEGIN_DEFINE_SPEC(FSBAtmosphericSafetyTestsSpec, "Sandbox.Character.AtmosphericSafety", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBAtmosphericSafetyComponent* AtmoComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBAtmosphericSafetyTestsSpec)

void FSBAtmosphericSafetyTestsSpec::Define()
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

		AtmoComp = NewObject<USBAtmosphericSafetyComponent>(TestActor, TEXT("AtmoComp"));
		TestActor->AddOwnedComponent(AtmoComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(AtmoComp);
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

	It("Should maintain 100% SpO2 in normal Earth atmosphere without hypoxia", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AtmoComp->SetupAtmosphericSafety(100.0f, 100.0f);

		FSBAtmosphereEnvironmentData NormalEnv;
		NormalEnv.OxygenPercentage = 21.0f;
		NormalEnv.BarometricPressureKPa = 101.3f;
		NormalEnv.bIsVacuum = false;

		AtmoComp->SimulateAtmosphereTick(5.0f, NormalEnv);

		TestEqual("Blood SpO2 is 100%", AtmoComp->GetAtmosphericSafetyData().BloodOxygenSaturation, 100.0f);
		TestFalse("Not hypoxic", AtmoComp->IsHypoxic());
		TestFalse("No Hypoxia tag", StateComp->HasTag(Tags.State_Atmosphere_Hypoxia));
		TestFalse("No Hazardous tag", StateComp->HasTag(Tags.State_Atmosphere_Hazardous));
	});

	It("Should deplete SpO2 in vacuum or low-O2 environment without suit and trigger Hypoxia tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AtmoComp->SetupAtmosphericSafety(100.0f, 100.0f);

		FSBAtmosphereEnvironmentData VacuumEnv;
		VacuumEnv.bIsVacuum = true;
		VacuumEnv.OxygenPercentage = 0.0f;

		USBCharacterTestListener* HypoxiaListener = NewObject<USBCharacterTestListener>(AtmoComp);
		AtmoComp->OnHypoxiaStateChanged.AddDynamic(HypoxiaListener, &USBCharacterTestListener::OnBool);

		AtmoComp->SimulateAtmosphereTick(3.0f, VacuumEnv);

		TestTrue("Hypoxia is active", AtmoComp->IsHypoxic());
		TestTrue("Hypoxia delegate fired with hypoxic state", HypoxiaListener->bFired && HypoxiaListener->BoolArg);
		TestTrue("Has Hypoxia tag", StateComp->HasTag(Tags.State_Atmosphere_Hypoxia));
		TestTrue("Has Hazardous tag", StateComp->HasTag(Tags.State_Atmosphere_Hazardous));
	});

	It("Should seal suit in vacuum, consume suit O2, restore SpO2 to 100%, and grant SuitPressurized tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AtmoComp->SetupAtmosphericSafety(100.0f, 100.0f);

		FSBAtmosphereEnvironmentData VacuumEnv;
		VacuumEnv.bIsVacuum = true;

		AtmoComp->SimulateAtmosphereTick(3.0f, VacuumEnv);
		TestTrue("Hypoxic before suit seal", AtmoComp->IsHypoxic());

		AtmoComp->ToggleSuitSeal(true);
		TestTrue("Has SuitPressurized tag", StateComp->HasTag(Tags.State_Atmosphere_SuitPressurized));

		AtmoComp->SimulateAtmosphereTick(8.0f, VacuumEnv);

		TestFalse("No longer hypoxic", AtmoComp->IsHypoxic());
		TestEqual("SpO2 restored to 100%", AtmoComp->GetAtmosphericSafetyData().BloodOxygenSaturation, 100.0f);
		TestTrue("Suit O2 consumed", AtmoComp->GetAtmosphericSafetyData().SuitOxygenReserve < 100.0f);
	});

	It("Should degrade filter in toxic gas atmosphere, trigger FilterExhausted and ToxicInhalation tags on depletion", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		AtmoComp->SetupAtmosphericSafety(100.0f, 20.0f);

		FSBAtmosphereEnvironmentData ToxicEnv;
		ToxicEnv.OxygenPercentage = 21.0f;
		ToxicEnv.ToxicGasPPM = 200.0f;

		USBCharacterTestListener* FilterListener = NewObject<USBCharacterTestListener>(AtmoComp);
		AtmoComp->OnFilterExhausted.AddDynamic(FilterListener, &USBCharacterTestListener::OnVoid);

		AtmoComp->SimulateAtmosphereTick(15.0f, ToxicEnv);

		TestTrue("Filter exhausted delegate fired", FilterListener->bFired);
		TestTrue("Has FilterExhausted tag", StateComp->HasTag(Tags.State_Atmosphere_FilterExhausted));
		TestTrue("Has ToxicInhalation tag", StateComp->HasTag(Tags.State_Atmosphere_ToxicInhalation));
		TestTrue("In toxic inhalation state", AtmoComp->IsInToxicInhalation());
	});
}
