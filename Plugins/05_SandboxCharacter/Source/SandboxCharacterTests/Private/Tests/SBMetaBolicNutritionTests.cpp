#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBMetaBolicNutritionComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "SBCharacterTestTypes.h"

BEGIN_DEFINE_SPEC(FSBMetaBolicNutritionTestsSpec, "Sandbox.Character.MetabolicNutrition", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBMetaBolicNutritionComponent* NutritionComp;
	USBStateComponent* CharacterStateComp;
END_DEFINE_SPEC(FSBMetaBolicNutritionTestsSpec)

void FSBMetaBolicNutritionTestsSpec::Define()
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

		NutritionComp = NewObject<USBMetaBolicNutritionComponent>(CharacterActor, TEXT("NutritionComp"));
		CharacterActor->AddOwnedComponent(NutritionComp);

		ISBComponentInterface::Execute_OnInitialize(CharacterStateComp);
		ISBComponentInterface::Execute_OnInitialize(NutritionComp);
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

	It("Should consume calories at base rate while resting and maintain WellFed tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		NutritionComp->SetupMetabolicNutrition(2000.0f, 100.0f);
		NutritionComp->SetActivityState(ESBMetabolicActivityState::Resting);
		NutritionComp->SimulateMetabolicTick(10.0f);

		TestNearlyEqual("Calories burned at base rate", NutritionComp->GetNutritionData().Calories, 1990.0f, 1.0f);
		TestEqual("Hunger level is Satiated", (int32)NutritionComp->GetHungerLevel(), (int32)ESBHungerLevel::Satiated);
		TestTrue("Has WellFed tag", CharacterStateComp->HasTag(Tags.State_Metabolism_WellFed));
	});

	It("Should accelerate calorie and hydration burn during high intensity combat and trigger Starvation and Dehydration", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		NutritionComp->SetupMetabolicNutrition(100.0f, 10.0f);
		NutritionComp->SetActivityState(ESBMetabolicActivityState::Combat);


		USBCharacterTestListener* StarvationListener = NewObject<USBCharacterTestListener>(NutritionComp);
		NutritionComp->OnStarvationTriggered.AddDynamic(StarvationListener, &USBCharacterTestListener::OnVoid);

		USBCharacterTestListener* DehydrationListener = NewObject<USBCharacterTestListener>(NutritionComp);
		NutritionComp->OnCriticalDehydrationTriggered.AddDynamic(DehydrationListener, &USBCharacterTestListener::OnVoid);

		NutritionComp->SimulateMetabolicTick(30.0f, 2.0f); // 4x combat mult * 2.0x heat = 8x hydration loss

		TestEqual("Calories completely depleted", NutritionComp->GetNutritionData().Calories, 0.0f);
		TestEqual("Hydration completely depleted", NutritionComp->GetNutritionData().Hydration, 0.0f);
		TestEqual("Hunger level is Starving", (int32)NutritionComp->GetHungerLevel(), (int32)ESBHungerLevel::Starving);
		TestEqual("Hydration level is CriticalDehydration", (int32)NutritionComp->GetHydrationLevel(), (int32)ESBHydrationLevel::CriticalDehydration);
		TestTrue("Starvation delegate fired", StarvationListener->bFired);
		TestTrue("Critical dehydration delegate fired", DehydrationListener->bFired);
		TestTrue("Has Starving tag", CharacterStateComp->HasTag(Tags.State_Metabolism_Starving));
		TestTrue("Has Dehydrated tag", CharacterStateComp->HasTag(Tags.State_Metabolism_Dehydrated));
	});

	It("Should restore calories, hydration, and vitamins when consuming food and drink, recovering to WellFed", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		NutritionComp->SetupMetabolicNutrition(100.0f, 20.0f);

		FSBConsumableNutritionItem MeatStew;
		MeatStew.CalorieYield = 1600.0f;
		MeatStew.HydrationYield = 60.0f;
		MeatStew.NutrientYield.VitaminC = 50.0f;

		NutritionComp->ConsumeFoodOrDrink(MeatStew);

		TestNearlyEqual("Calories restored to 1700", NutritionComp->GetNutritionData().Calories, 1700.0f, 1.0f);
		TestNearlyEqual("Hydration restored to 80%", NutritionComp->GetNutritionData().Hydration, 80.0f, 1.0f);
		TestEqual("Hunger level restored to Satiated", (int32)NutritionComp->GetHungerLevel(), (int32)ESBHungerLevel::Satiated);
		TestEqual("Hydration level restored to Hydrated", (int32)NutritionComp->GetHydrationLevel(), (int32)ESBHydrationLevel::Hydrated);
		TestTrue("Has WellFed tag", CharacterStateComp->HasTag(Tags.State_Metabolism_WellFed));
		TestTrue("Has Hydrated tag", CharacterStateComp->HasTag(Tags.State_Metabolism_Hydrated));
	});

	It("Should trigger nutrient deficiency delegate and tags on prolonged vitamin C and electrolyte depletion", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		NutritionComp->SetupMetabolicNutrition(2000.0f, 100.0f);


		USBCharacterTestListener* DeficiencyListener = NewObject<USBCharacterTestListener>(NutritionComp);
		NutritionComp->OnNutrientDeficiencyTriggered.AddDynamic(DeficiencyListener, &USBCharacterTestListener::OnName);

		NutritionComp->SetActivityState(ESBMetabolicActivityState::Combat);
		NutritionComp->SimulateMetabolicTick(4500.0f); // 4500 * 0.02 = 90% depletion

		TestTrue("Vitamin C deficiency is active", NutritionComp->HasDeficiency(FName("VitaminC")));
		TestTrue("Electrolyte deficiency is active", NutritionComp->HasDeficiency(FName("Electrolytes")));
		TestTrue("Vitamin C deficiency delegate fired", DeficiencyListener->NameArgs.Contains(FName("VitaminC")));
		TestTrue("Electrolyte deficiency delegate fired", DeficiencyListener->NameArgs.Contains(FName("Electrolytes")));
		TestTrue("Has Vitamin C deficiency tag", CharacterStateComp->HasTag(Tags.State_Metabolism_Deficiency_VitaminC));
		TestTrue("Has Electrolyte deficiency tag", CharacterStateComp->HasTag(Tags.State_Metabolism_Deficiency_Electrolytes));
	});
}
