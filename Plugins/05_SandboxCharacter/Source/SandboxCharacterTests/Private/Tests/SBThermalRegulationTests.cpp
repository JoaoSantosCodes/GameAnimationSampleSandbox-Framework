#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBThermalRegulationComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "SBCharacterTestTypes.h"

BEGIN_DEFINE_SPEC(FSBThermalRegulationTestsSpec, "Sandbox.Character.ThermalRegulation", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBThermalRegulationComponent* ThermalComp;
	USBStateComponent* CharacterStateComp;
END_DEFINE_SPEC(FSBThermalRegulationTestsSpec)

void FSBThermalRegulationTestsSpec::Define()
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

		CharacterStateComp = NewObject<USBStateComponent>(CharacterActor, TEXT("CharacterStateComp"));
		CharacterActor->AddOwnedComponent(CharacterStateComp);

		ThermalComp = NewObject<USBThermalRegulationComponent>(CharacterActor, TEXT("ThermalComp"));
		CharacterActor->AddOwnedComponent(ThermalComp);

		ISBComponentInterface::Execute_OnInitialize(CharacterStateComp);
		ISBComponentInterface::Execute_OnInitialize(ThermalComp);
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

	It("Should maintain 37°C homeostasis in temperate comfortable environment and grant Comfortable tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ThermalComp->SetupThermalRegulation(37.0f, 22.0f);
		ThermalComp->SimulateThermalTick(5.0f);

		TestNearlyEqual("Core temperature maintained at 37°C", ThermalComp->GetThermalData().CoreTemperature, 37.0f, 0.2f);
		TestEqual("Comfort state is Comfortable", (int32)ThermalComp->GetComfortState(), (int32)ESBThermalComfortState::Comfortable);
		TestTrue("Has Comfortable tag", CharacterStateComp->HasTag(Tags.State_Thermal_Comfortable));
	});

	It("Should lose body heat in freezing sub-zero blizzard, drop below 35°C, and trigger CriticalHypothermia", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ThermalComp->SetupThermalRegulation(37.0f, -25.0f);
		ThermalComp->SetWindChill(10.0f);
		ThermalComp->SetWetnessLevel(0.5f);

		USBCharacterTestListener* HypothermiaListener = NewObject<USBCharacterTestListener>(ThermalComp);
		ThermalComp->OnHypothermiaTriggered.AddDynamic(HypothermiaListener, &USBCharacterTestListener::OnFloat);

		ThermalComp->SimulateThermalTick(2.0f);

		TestTrue("Core temperature dropped below 35°C", ThermalComp->GetThermalData().CoreTemperature < 35.0f);
		TestEqual("Comfort state is CriticalHypothermia", (int32)ThermalComp->GetComfortState(), (int32)ESBThermalComfortState::CriticalHypothermia);
		TestTrue("OnHypothermiaTriggered delegate fired", HypothermiaListener->bFired);
		TestTrue("Has Hypothermia tag", CharacterStateComp->HasTag(Tags.State_Thermal_Hypothermia));
	});

	It("Should protect body temperature in freezing conditions with thermal insulation and nearby campfire", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ThermalComp->SetupThermalRegulation(37.0f, -10.0f);
		ThermalComp->SetThermalInsulation(0.9f, 0.0f);
		ThermalComp->SetNearbyHeatSource(35.0f); // Effective = +25°C

		ThermalComp->SimulateThermalTick(5.0f);

		TestNearlyEqual("Core temperature protected near 37°C", ThermalComp->GetThermalData().CoreTemperature, 37.0f, 0.5f);
		TestEqual("Comfort state remains Comfortable", (int32)ThermalComp->GetComfortState(), (int32)ESBThermalComfortState::Comfortable);
		TestTrue("Has Comfortable tag", CharacterStateComp->HasTag(Tags.State_Thermal_Comfortable));
	});

	It("Should gain body heat in scorching desert heat, exceed 39.5°C, and trigger CriticalHeatstroke", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ThermalComp->SetupThermalRegulation(37.0f, 52.0f);

		USBCharacterTestListener* HeatstrokeListener = NewObject<USBCharacterTestListener>(ThermalComp);
		ThermalComp->OnHeatstrokeTriggered.AddDynamic(HeatstrokeListener, &USBCharacterTestListener::OnFloat);

		ThermalComp->SimulateThermalTick(4.0f);

		TestTrue("Core temperature elevated above 39.5°C", ThermalComp->GetThermalData().CoreTemperature > 39.5f);
		TestEqual("Comfort state is CriticalHeatstroke", (int32)ThermalComp->GetComfortState(), (int32)ESBThermalComfortState::CriticalHeatstroke);
		TestTrue("OnHeatstrokeTriggered delegate fired", HeatstrokeListener->bFired);
		TestTrue("Has Heatstroke tag", CharacterStateComp->HasTag(Tags.State_Thermal_Heatstroke));
	});
}
