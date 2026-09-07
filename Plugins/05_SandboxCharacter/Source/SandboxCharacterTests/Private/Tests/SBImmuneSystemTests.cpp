// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBImmuneSystemComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "SBCharacterTestTypes.h"

BEGIN_DEFINE_SPEC(FSBImmuneSystemTestsSpec, "Sandbox.Character.ImmuneSystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBImmuneSystemComponent* ImmuneComp;
	USBStateComponent* CharacterStateComp;
END_DEFINE_SPEC(FSBImmuneSystemTestsSpec)

void FSBImmuneSystemTestsSpec::Define()
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

		ImmuneComp = NewObject<USBImmuneSystemComponent>(CharacterActor, TEXT("ImmuneComp"));
		CharacterActor->AddOwnedComponent(ImmuneComp);

		ISBComponentInterface::Execute_OnInitialize(CharacterStateComp);
		ISBComponentInterface::Execute_OnInitialize(ImmuneComp);
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

	It("Should expose to pathogen, enter Incubating stage, and apply Incubating tag without fever", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ImmuneComp->SetupImmuneSystem(1.0f);

		FSBPathogenStrain Bacteria;
		Bacteria.PathogenID = FName("WoundInfection");
		Bacteria.Virulence = 0.5f;
		Bacteria.IncubationThreshold = 25.0f;

		bool bExposed = ImmuneComp->ExposeToPathogen(Bacteria, 5.0f);

		TestTrue("Exposed to pathogen successfully", bExposed);
		TestTrue("IsInfectedWith returned true", ImmuneComp->IsInfectedWith(FName("WoundInfection")));
		TestEqual("Stage is Incubating", (int32)ImmuneComp->GetInfectionStage(FName("WoundInfection")), (int32)ESBInfectionStage::Incubating);
		TestTrue("Has Infected tag", CharacterStateComp->HasTag(Tags.State_Immunity_Infected));
		TestTrue("Has Incubating tag", CharacterStateComp->HasTag(Tags.State_Immunity_Incubating));
		TestFalse("No fever yet", CharacterStateComp->HasTag(Tags.State_Immunity_Fever));
	});

	It("Should advance pathogen load past incubation threshold, enter Symptomatic stage, and trigger Fever", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ImmuneComp->SetupImmuneSystem(0.2f);

		FSBPathogenStrain Flu;
		Flu.PathogenID = FName("RespiratoryFlu");
		Flu.Virulence = 5.0f;
		Flu.IncubationThreshold = 20.0f;

		ImmuneComp->ExposeToPathogen(Flu, 15.0f);

		USBCharacterTestListener* FeverListener = NewObject<USBCharacterTestListener>(ImmuneComp);
		ImmuneComp->OnFeverTriggered.AddDynamic(FeverListener, &USBCharacterTestListener::OnFloat);

		ImmuneComp->SimulateImmuneTick(3.0f);

		TestEqual("Stage transitioned to Symptomatic", (int32)ImmuneComp->GetInfectionStage(FName("RespiratoryFlu")), (int32)ESBInfectionStage::Symptomatic);
		TestEqual("Fever offset elevated to 2°C", ImmuneComp->GetTotalFeverOffset(), 2.0f);
		TestTrue("Fever delegate fired", FeverListener->bFired);
		TestTrue("Has Symptomatic tag", CharacterStateComp->HasTag(Tags.State_Immunity_Symptomatic));
		TestTrue("Has Fever tag", CharacterStateComp->HasTag(Tags.State_Immunity_Fever));
	});

	It("Should apply medical treatment to suppress pathogen load, enter Recovering stage, and cure infection", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ImmuneComp->SetupImmuneSystem(0.5f);

		FSBPathogenStrain Plague;
		Plague.PathogenID = FName("AlienPlague");
		Plague.Virulence = 2.0f;
		Plague.IncubationThreshold = 20.0f;

		ImmuneComp->ExposeToPathogen(Plague, 30.0f);

		USBCharacterTestListener* CuredListener = NewObject<USBCharacterTestListener>(ImmuneComp);
		ImmuneComp->OnInfectionCured.AddDynamic(CuredListener, &USBCharacterTestListener::OnName);

		ImmuneComp->ApplyMedicalTreatment(FName("AlienPlague"), 20.0f);
		ImmuneComp->SimulateImmuneTick(1.0f);

		TestTrue("Infection cured delegate fired", CuredListener->bFired);
		TestFalse("No longer infected", ImmuneComp->IsInfectedWith(FName("AlienPlague")));
		TestFalse("Infected tag cleared", CharacterStateComp->HasTag(Tags.State_Immunity_Infected));
	});

	It("Should build natural antibodies, overcome infection, and grant permanent adaptive immunity", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ImmuneComp->SetupImmuneSystem(2.0f);

		FSBPathogenStrain MinorVirus;
		MinorVirus.PathogenID = FName("MinorCold");
		MinorVirus.Virulence = 0.5f;
		MinorVirus.IncubationThreshold = 30.0f;

		ImmuneComp->ExposeToPathogen(MinorVirus, 10.0f);

		USBCharacterTestListener* ImmunityListener = NewObject<USBCharacterTestListener>(ImmuneComp);
		ImmuneComp->OnImmunityAcquired.AddDynamic(ImmunityListener, &USBCharacterTestListener::OnName);

		ImmuneComp->SimulateImmuneTick(10.0f);

		TestTrue("Immunity acquired delegate fired", ImmunityListener->bFired);
		TestTrue("Has Immune tag", CharacterStateComp->HasTag(Tags.State_Immunity_Immune));

		bool bReInfect = ImmuneComp->ExposeToPathogen(MinorVirus, 10.0f);
		TestFalse("Immune system rejected secondary infection", bReInfect);
	});
}
