#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBResourceExtractorComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBResourceExtractorTestsSpec, "Sandbox.Inventory.ResourceExtractor", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* ExtractorActor;
	USBResourceExtractorComponent* ExtractorComp;
	USBStateComponent* ExtractorStateComp;
END_DEFINE_SPEC(FSBResourceExtractorTestsSpec)

void FSBResourceExtractorTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ExtractorActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(ExtractorActor, TEXT("Root"));
		ExtractorActor->SetRootComponent(Root);
		Root->RegisterComponent();

		ExtractorStateComp = NewObject<USBStateComponent>(ExtractorActor, TEXT("ExtractorStateComp"));
		ExtractorActor->AddOwnedComponent(ExtractorStateComp);

		ExtractorComp = NewObject<USBResourceExtractorComponent>(ExtractorActor, TEXT("ExtractorComp"));
		ExtractorActor->AddOwnedComponent(ExtractorComp);

		ISBComponentInterface::Execute_OnInitialize(ExtractorStateComp);
		ISBComponentInterface::Execute_OnInitialize(ExtractorComp);
	});

	AfterEach([this]()
	{
		if (ExtractorActor)
		{
			ExtractorActor->Destroy();
			ExtractorActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should extract resources at pure deposit rate (2x), accumulate output items, and grant Drilling tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ExtractorComp->SetupExtractor(ESBExtractorType::MiningDrill, FName(TEXT("IronOre")), ESBFluidType::None, ESBResourceDepositPurity::Pure, 1.0f, 15.0f);
		ExtractorComp->SetPowerSupplied(true);

		TestEqual("Extractor state is Extracting", (int32)ExtractorComp->GetExtractorState(), (int32)ESBExtractorState::Extracting);
		TestTrue("Extractor has Drilling tag", ExtractorStateComp->HasTag(Tags.State_Extractor_Drilling));

		// Tick 1.0s at 2.0x speed -> 2 items extracted
		ExtractorComp->SimulateExtractorTick(1.0f);

		TestEqual("Extracted 2 items", ExtractorComp->GetExtractorData().TotalExtractedItems, 2);
		TestEqual("Output buffer has 2 IronOre", ExtractorComp->GetOutputItemCount(FName(TEXT("IronOre"))), 2);
	});

	It("Should halt extraction immediately when power is lost, transition to NoPower, and grant NoPower tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ExtractorComp->SetupExtractor(ESBExtractorType::MiningDrill, FName(TEXT("IronOre")), ESBFluidType::None, ESBResourceDepositPurity::Pure, 1.0f, 15.0f);
		ExtractorComp->SetPowerSupplied(true);
		ExtractorComp->SimulateExtractorTick(1.0f);

		// Cut power
		ExtractorComp->SetPowerSupplied(false);

		TestEqual("State is NoPower", (int32)ExtractorComp->GetExtractorState(), (int32)ESBExtractorState::NoPower);
		TestTrue("Has NoPower tag", ExtractorStateComp->HasTag(Tags.State_Extractor_NoPower));

		ExtractorComp->SimulateExtractorTick(1.0f);
		TestEqual("Total extracted items did not advance", ExtractorComp->GetExtractorData().TotalExtractedItems, 2);
	});

	It("Should transition to Depleted state and halt when deposit is marked depleted", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ExtractorComp->SetupExtractor(ESBExtractorType::MiningDrill, FName(TEXT("IronOre")), ESBFluidType::None, ESBResourceDepositPurity::Normal, 1.0f, 15.0f);
		ExtractorComp->SetPowerSupplied(true);

		// Mark depleted
		ExtractorComp->SetDepositDepleted(true);

		TestEqual("State is Depleted", (int32)ExtractorComp->GetExtractorState(), (int32)ESBExtractorState::Depleted);
		TestTrue("Has Depleted tag", ExtractorStateComp->HasTag(Tags.State_Extractor_Depleted));

		ExtractorComp->SimulateExtractorTick(1.0f);
		TestEqual("No items extracted while depleted", ExtractorComp->GetExtractorData().TotalExtractedItems, 0);
	});

	It("Should block extraction when output buffer reaches capacity and resume when harvested", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		ExtractorComp->Settings.OutputItemBufferCapacity = 3;
		ExtractorComp->SetupExtractor(ESBExtractorType::MiningDrill, FName(TEXT("IronOre")), ESBFluidType::None, ESBResourceDepositPurity::Normal, 1.0f, 15.0f);
		ExtractorComp->SetPowerSupplied(true);

		// Tick 3.0s to fill capacity of 3 items
		ExtractorComp->SimulateExtractorTick(1.0f);
		ExtractorComp->SimulateExtractorTick(1.0f);
		ExtractorComp->SimulateExtractorTick(1.0f);
		TestEqual("Output buffer has 3 IronOre", ExtractorComp->GetOutputItemCount(FName(TEXT("IronOre"))), 3);

		// Next tick should detect OutputBlocked
		ExtractorComp->SimulateExtractorTick(0.1f);
		TestEqual("State is OutputBlocked", (int32)ExtractorComp->GetExtractorState(), (int32)ESBExtractorState::OutputBlocked);
		TestTrue("Has OutputBlocked tag", ExtractorStateComp->HasTag(Tags.State_Extractor_OutputBlocked));

		// Withdraw 1 item to make room
		int32 Extracted = ExtractorComp->WithdrawOutputItem(FName(TEXT("IronOre")), 1);
		TestEqual("Withdrawn 1 item", Extracted, 1);
		TestEqual("State resumed to Extracting", (int32)ExtractorComp->GetExtractorState(), (int32)ESBExtractorState::Extracting);
		TestTrue("Has Drilling tag", ExtractorStateComp->HasTag(Tags.State_Extractor_Drilling));
	});
}
