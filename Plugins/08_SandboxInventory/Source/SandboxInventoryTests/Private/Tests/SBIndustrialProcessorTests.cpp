#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBIndustrialProcessorComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBIndustrialProcessorTestsSpec, "Sandbox.Inventory.IndustrialProcessor", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* SmelterActor;
	USBIndustrialProcessorComponent* SmelterComp;
	USBStateComponent* SmelterStateComp;
	FSBIndustrialRecipe IronSmeltRecipe;
END_DEFINE_SPEC(FSBIndustrialProcessorTestsSpec)

void FSBIndustrialProcessorTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SmelterActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(SmelterActor, TEXT("Root"));
		SmelterActor->SetRootComponent(Root);
		Root->RegisterComponent();

		SmelterStateComp = NewObject<USBStateComponent>(SmelterActor, TEXT("SmelterStateComp"));
		SmelterActor->AddOwnedComponent(SmelterStateComp);

		SmelterComp = NewObject<USBIndustrialProcessorComponent>(SmelterActor, TEXT("SmelterComp"));
		SmelterActor->AddOwnedComponent(SmelterComp);

		// IronSmeltRecipe e membro do spec e o BeforeEach roda antes de CADA It. Sem este
		// reset, InputItems/OutputItems acumulavam a cada teste (1 ingrediente, depois 2,
		// depois 3), fazendo o forno consumir e produzir multiplo do esperado.
		IronSmeltRecipe = FSBIndustrialRecipe();

		IronSmeltRecipe.RecipeId = FName(TEXT("SmeltIron"));
		IronSmeltRecipe.DisplayName = FText::FromString(TEXT("Smelt Iron Ingot"));
		IronSmeltRecipe.CraftingTime = 1.0f;
		IronSmeltRecipe.PowerRequirement = 20.0f;
		IronSmeltRecipe.HeatGeneration = 5.0f;

		FSBIndustrialIngredient InIng;
		InIng.ItemId = FName(TEXT("IronOre"));
		InIng.Quantity = 1;
		IronSmeltRecipe.InputItems.Add(InIng);

		FSBIndustrialIngredient OutIng;
		OutIng.ItemId = FName(TEXT("IronIngot"));
		OutIng.Quantity = 1;
		IronSmeltRecipe.OutputItems.Add(OutIng);

		ISBComponentInterface::Execute_OnInitialize(SmelterStateComp);
		ISBComponentInterface::Execute_OnInitialize(SmelterComp);
	});

	AfterEach([this]()
	{
		if (SmelterActor)
		{
			SmelterActor->Destroy();
			SmelterActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should process recipe cycle with power and ingredients, produce output items, and grant Processing tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SmelterComp->SetupProcessor(ESBProcessorType::Smelter, IronSmeltRecipe);
		SmelterComp->SetPowerSupplied(true);
		SmelterComp->DepositInputItem(FName(TEXT("IronOre")), 10);

		TestEqual("Smelter state is Processing", (int32)SmelterComp->GetProcessorState(), (int32)ESBProcessorState::Processing);
		TestTrue("Smelter has Processing tag", SmelterStateComp->HasTag(Tags.State_Industrial_Processing));

		// Tick 1.0s to complete 1 cycle
		SmelterComp->SimulateProcessorTick(1.0f);

		TestEqual("Completed 1 cycle", SmelterComp->GetProcessorData().CompletedCyclesCount, 1);
		TestEqual("Remaining IronOre is 9", SmelterComp->GetInputItemCount(FName(TEXT("IronOre"))), 9);
		TestEqual("Output IronIngot is 1", SmelterComp->GetOutputItemCount(FName(TEXT("IronIngot"))), 1);
	});

	It("Should halt production immediately when power supply is cut, transition to NoPower, and grant NoPower tag", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SmelterComp->SetupProcessor(ESBProcessorType::Smelter, IronSmeltRecipe);
		SmelterComp->SetPowerSupplied(true);
		SmelterComp->DepositInputItem(FName(TEXT("IronOre")), 5);

		// Cut power
		SmelterComp->SetPowerSupplied(false);

		TestEqual("State is NoPower", (int32)SmelterComp->GetProcessorState(), (int32)ESBProcessorState::NoPower);
		TestTrue("Has NoPower tag", SmelterStateComp->HasTag(Tags.State_Industrial_NoPower));

		SmelterComp->SimulateProcessorTick(1.0f);
		TestEqual("Cycle count did not advance", SmelterComp->GetProcessorData().CompletedCyclesCount, 0);
		TestEqual("Input items not consumed", SmelterComp->GetInputItemCount(FName(TEXT("IronOre"))), 5);
	});

	It("Should stop processing and enter MissingIngredients state when inputs are depleted", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SmelterComp->SetupProcessor(ESBProcessorType::Smelter, IronSmeltRecipe);
		SmelterComp->SetPowerSupplied(true);
		// No ingredients deposited yet

		TestEqual("State is MissingIngredients", (int32)SmelterComp->GetProcessorState(), (int32)ESBProcessorState::MissingIngredients);
		TestTrue("Has MissingIngredients tag", SmelterStateComp->HasTag(Tags.State_Industrial_MissingIngredients));

		// Deposit ingredients -> resumes processing immediately
		SmelterComp->DepositInputItem(FName(TEXT("IronOre")), 2);
		TestEqual("State resumed to Processing", (int32)SmelterComp->GetProcessorState(), (int32)ESBProcessorState::Processing);
		TestTrue("Has Processing tag", SmelterStateComp->HasTag(Tags.State_Industrial_Processing));
	});

	It("Should stall production when output buffer is full and resume when cleared", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		SmelterComp->Settings.OutputItemBufferCapacity = 2;
		SmelterComp->SetupProcessor(ESBProcessorType::Smelter, IronSmeltRecipe);
		SmelterComp->SetPowerSupplied(true);
		SmelterComp->DepositInputItem(FName(TEXT("IronOre")), 10);

		// Complete 2 cycles to fill output buffer
		SmelterComp->SimulateProcessorTick(1.0f);
		SmelterComp->SimulateProcessorTick(1.0f);
		TestEqual("Produced 2 IronIngot", SmelterComp->GetOutputItemCount(FName(TEXT("IronIngot"))), 2);

		// Next tick should detect OutputFull
		SmelterComp->SimulateProcessorTick(0.1f);
		TestEqual("State is OutputFull", (int32)SmelterComp->GetProcessorState(), (int32)ESBProcessorState::OutputFull);
		TestTrue("Has OutputFull tag", SmelterStateComp->HasTag(Tags.State_Industrial_OutputFull));

		// Withdraw 1 item to make room
		int32 Withdrawn = SmelterComp->WithdrawOutputItem(FName(TEXT("IronIngot")), 1);
		TestEqual("Withdrawn 1 item", Withdrawn, 1);
		TestEqual("State resumed to Processing", (int32)SmelterComp->GetProcessorState(), (int32)ESBProcessorState::Processing);
	});
}
