#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBDynamicCropComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "SBInventoryTestTypes.h"

BEGIN_DEFINE_SPEC(FSBDynamicCropTestsSpec, "Sandbox.Inventory.DynamicCrop", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CropPlotActor;
	USBDynamicCropComponent* CropComp;
	USBStateComponent* CropStateComp;
END_DEFINE_SPEC(FSBDynamicCropTestsSpec)

void FSBDynamicCropTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CropPlotActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(CropPlotActor, TEXT("Root"));
		CropPlotActor->SetRootComponent(Root);
		Root->RegisterComponent();

		CropStateComp = NewObject<USBStateComponent>(CropPlotActor, TEXT("CropStateComp"));
		CropPlotActor->AddOwnedComponent(CropStateComp);

		CropComp = NewObject<USBDynamicCropComponent>(CropPlotActor, TEXT("CropComp"));
		CropPlotActor->AddOwnedComponent(CropComp);

		ISBComponentInterface::Execute_OnInitialize(CropStateComp);
		ISBComponentInterface::Execute_OnInitialize(CropComp);
	});

	AfterEach([this]()
	{
		if (CropPlotActor)
		{
			CropPlotActor->Destroy();
			CropPlotActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should plant seed in moist soil, enter Seeded stage, and apply Fertilized tag on enrichment", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		CropComp->SetupCropPlot(0.6f, 1.0f);

		FSBPlantSpeciesData Wheat;
		Wheat.SpeciesID = FName("StarlightWheat");
		Wheat.GrowthDuration = 60.0f;
		Wheat.OptimalMoisture = 0.6f;

		bool bPlanted = CropComp->PlantSeed(Wheat);

		TestTrue("PlantSeed succeeded", bPlanted);
		TestEqual("Stage is Seeded", (int32)CropComp->GetGrowthStage(), (int32)ESBCropGrowthStage::Seeded);
		TestTrue("Has Seeded tag", CropStateComp->HasTag(Tags.State_Crop_Seeded));

		CropComp->ApplyFertilizer(0.5f);
		TestTrue("Has Fertilized tag", CropStateComp->HasTag(Tags.State_Crop_Fertilized));
		TestEqual("Fertility updated", CropComp->GetCropData().SoilFertility, 1.5f);
	});

	It("Should advance growth progress with adequate moisture and transition through stages to Harvestable", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		CropComp->SetupCropPlot(0.5f, 1.0f);

		FSBPlantSpeciesData Berry;
		Berry.SpeciesID = FName("SolarBerry");
		Berry.GrowthDuration = 10.0f;
		Berry.OptimalMoisture = 0.5f;
		Berry.WaterConsumptionRate = 0.01f;

		CropComp->PlantSeed(Berry);
		CropComp->SimulateCropTick(5.0f, 1.0f, 22.0f);

		TestEqual("Reached Vegetative stage", (int32)CropComp->GetGrowthStage(), (int32)ESBCropGrowthStage::Vegetative);
		TestTrue("Has Growing tag", CropStateComp->HasTag(Tags.State_Crop_Growing));

		// MoistureFactor = 1 - |umidade - ótima|, ou seja penaliza excesso tanto quanto falta.
		// Regar +0.5 sobre 0.4 levava a umidade a 0.9 contra ótima de 0.5, DERRUBANDO o fator
		// de 0.90 para 0.72 -- a rega atrasava o crescimento. Repor até a ótima e dar tempo
		// suficiente: GrowthDuration=10 é o tempo em condição perfeita, e o fator nunca é 1.0
		// enquanto a umidade drena durante o tick.
		CropComp->WaterSoil(0.1f);
		CropComp->SimulateCropTick(7.0f, 1.0f, 22.0f);

		TestEqual("Reached Harvestable stage", (int32)CropComp->GetGrowthStage(), (int32)ESBCropGrowthStage::Harvestable);
		TestTrue("Has Harvestable tag", CropStateComp->HasTag(Tags.State_Crop_Harvestable));
		TestTrue("IsHarvestable returned true", CropComp->IsHarvestable());
	});

	It("Should harvest mature crop, yield produce with fertility multiplier, and broadcast delegate", [this]()
	{
		CropComp->SetupCropPlot(0.5f, 1.5f);

		FSBPlantSpeciesData Corn;
		Corn.SpeciesID = FName("AstraCorn");
		Corn.BaseYield = 4;
		Corn.GrowthDuration = 1.0f;
		Corn.bIsPerennial = true;

		CropComp->PlantSeed(Corn);
		CropComp->SimulateCropTick(2.0f);

		TestTrue("Crop is ready for harvest", CropComp->IsHarvestable());

		int32 HarvestYield = 0;
		USBInventoryTestListener* HarvestListener = NewObject<USBInventoryTestListener>(CropComp);
		CropComp->OnCropHarvested.AddDynamic(HarvestListener, &USBInventoryTestListener::OnNameInt);

		bool bHarvested = CropComp->HarvestCrop(HarvestYield);

		TestTrue("Harvest succeeded", bHarvested);
		TestTrue("Harvest delegate fired for AstraCorn", HarvestListener->bFired && HarvestListener->NameArg == FName("AstraCorn"));
		TestEqual("Yield was multiplied by fertility", HarvestYield, 6);
		TestEqual("Perennial plant reset to Vegetative stage", (int32)CropComp->GetGrowthStage(), (int32)ESBCropGrowthStage::Vegetative);
	});

	It("Should wither and die when experiencing severe drought under scorching heat and broadcast delegate", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		CropComp->SetupCropPlot(0.0f, 1.0f);

		FSBPlantSpeciesData Herb;
		Herb.SpeciesID = FName("LunarHerb");
		Herb.GrowthDuration = 10.0f;

		CropComp->PlantSeed(Herb);

		USBInventoryTestListener* WitherListener = NewObject<USBInventoryTestListener>(CropComp);
		CropComp->OnCropWithered.AddDynamic(WitherListener, &USBInventoryTestListener::OnName);

		CropComp->SimulateCropTick(1.0f, 1.0f, 40.0f);

		TestTrue("Wither delegate fired for LunarHerb", WitherListener->bFired && WitherListener->NameArg == FName("LunarHerb"));
		TestTrue("Is withered", CropComp->IsWithered());
		TestTrue("Has Withered tag", CropStateComp->HasTag(Tags.State_Crop_Withered));
	});
}
