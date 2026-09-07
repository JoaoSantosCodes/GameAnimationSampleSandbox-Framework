#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBDomesticationComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "SBCharacterTestTypes.h"

BEGIN_DEFINE_SPEC(FSBDomesticationTestsSpec, "Sandbox.Character.Domestication", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* FaunaActor;
	USBDomesticationComponent* DomComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBDomesticationTestsSpec)

void FSBDomesticationTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FaunaActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(FaunaActor, TEXT("Root"));
		FaunaActor->SetRootComponent(Root);
		Root->RegisterComponent();

		StateComp = NewObject<USBStateComponent>(FaunaActor, TEXT("StateComp"));
		FaunaActor->AddOwnedComponent(StateComp);

		DomComp = NewObject<USBDomesticationComponent>(FaunaActor, TEXT("DomComp"));
		FaunaActor->AddOwnedComponent(DomComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(DomComp);
	});

	AfterEach([this]()
	{
		if (FaunaActor)
		{
			FaunaActor->Destroy();
			FaunaActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should feed preferred food advancing taming progress and tame wild creature at 100%", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBCreatureGenetics Genetics;
		DomComp->SetupFauna(FName("SweetRoot"), Genetics);

		TestEqual("Initial state is Wild", (int32)DomComp->GetDomesticationData().DomesticationState, (int32)ESBFaunaDomesticationState::Wild);
		TestTrue("Has Wild tag", StateComp->HasTag(Tags.State_Fauna_Wild));

		DomComp->FeedTamingFood(FName("SweetRoot"), 5.0f);
		TestEqual("State transitioned to Taming", (int32)DomComp->GetDomesticationData().DomesticationState, (int32)ESBFaunaDomesticationState::Taming);
		TestTrue("Has Taming tag", StateComp->HasTag(Tags.State_Fauna_Taming));

		USBCharacterTestListener* TamedListener = NewObject<USBCharacterTestListener>(DomComp);
		DomComp->OnCreatureTamed.AddDynamic(TamedListener, &USBCharacterTestListener::OnVoid);

		DomComp->FeedTamingFood(FName("SweetRoot"), 5.0f);
		TestTrue("Creature is fully domesticated", DomComp->IsDomesticated());
		TestTrue("OnCreatureTamed delegate fired", TamedListener->bFired);
		TestTrue("Has Domesticated tag", StateComp->HasTag(Tags.State_Fauna_Domesticated));
	});

	It("Should increase affection when petting domesticated creature and grant Mountable tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBCreatureGenetics Genetics;
		DomComp->SetupFauna(FName("SweetRoot"), Genetics);
		DomComp->FeedTamingFood(FName("SweetRoot"), 10.0f);

		TestTrue("Creature is domesticated", DomComp->IsDomesticated());
		TestFalse("Not mountable yet", StateComp->HasTag(Tags.State_Fauna_Mountable));

		DomComp->PetCreature(0.4f);
		TestTrue("Has Mountable tag after affection boost", StateComp->HasTag(Tags.State_Fauna_Mountable));
		TestTrue("Affection level >= 0.8", DomComp->GetDomesticationData().AffectionLevel >= 0.8f);
	});

	It("Should start breeding, enter Pregnant stage with tag, advance pregnancy on tick, and complete gestation", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBCreatureGenetics Genetics;
		DomComp->SetupFauna(FName("SweetRoot"), Genetics);
		DomComp->FeedTamingFood(FName("SweetRoot"), 10.0f);

		FSBCreatureGenetics PartnerGenetics;
		PartnerGenetics.SpeedModifier = 1.2f;

		bool bBred = DomComp->StartBreedingWith(PartnerGenetics, 10.0f);
		TestTrue("Breeding started successfully", bBred);
		TestTrue("Is pregnant", DomComp->IsPregnant());
		TestTrue("Has Pregnant tag", StateComp->HasTag(Tags.State_Fauna_Pregnant));

		USBCharacterTestListener* PregnancyListener = NewObject<USBCharacterTestListener>(DomComp);
		DomComp->OnPregnancyCompleted.AddDynamic(PregnancyListener, &USBCharacterTestListener::OnVoid);

		DomComp->SimulateFaunaTick(10.0f);
		TestTrue("Pregnancy completed delegate fired", PregnancyListener->bFired);
		TestEqual("Stage is OffspringReady", (int32)DomComp->GetDomesticationData().ReproductiveStage, (int32)ESBFaunaReproductiveStage::OffspringReady);
	});

	It("Should birth offspring inheriting combined parental genetics and increment generation", [this]()
	{
		FSBCreatureGenetics Genetics;
		Genetics.SpeedModifier = 1.0f;
		Genetics.StaminaModifier = 1.0f;
		Genetics.Generation = 1;

		DomComp->SetupFauna(FName("SweetRoot"), Genetics);
		DomComp->FeedTamingFood(FName("SweetRoot"), 10.0f);

		FSBCreatureGenetics PartnerGenetics;
		PartnerGenetics.SpeedModifier = 1.2f;
		PartnerGenetics.StaminaModifier = 1.4f;
		PartnerGenetics.Generation = 1;

		DomComp->StartBreedingWith(PartnerGenetics, 5.0f);
		DomComp->SimulateFaunaTick(5.0f);

		FSBCreatureGenetics Offspring;
		USBCharacterTestListener* BirthListener = NewObject<USBCharacterTestListener>(DomComp);
		DomComp->OnOffspringBirthed.AddDynamic(BirthListener, &USBCharacterTestListener::OnGenetics);

		bool bBirthed = DomComp->BirthOffspring(Offspring);
		TestTrue("Birth succeeded", bBirthed);
		TestTrue("Birth delegate fired", BirthListener->bFired);
		TestEqual("Offspring generation is 2", Offspring.Generation, 2);
		TestTrue("Speed modifier combines parental average with mutation boost", Offspring.SpeedModifier > 1.1f);
		TestEqual("Reproductive stage reset to NonBreeding", (int32)DomComp->GetDomesticationData().ReproductiveStage, (int32)ESBFaunaReproductiveStage::NonBreeding);
	});
}
