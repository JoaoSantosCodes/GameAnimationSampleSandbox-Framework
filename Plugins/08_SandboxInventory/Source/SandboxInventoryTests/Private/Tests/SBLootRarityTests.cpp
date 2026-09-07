#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Actors/SBPhysicalLootDrop.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment_Rarity.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBLootRarityTestsSpec, "Sandbox.Inventory.LootRarity", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBPhysicalLootDrop* TestDrop;
	USBItemDefinition* CommonItemDef;
	USBItemDefinition* LegendaryItemDef;
END_DEFINE_SPEC(FSBLootRarityTestsSpec)

void FSBLootRarityTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		UGameInstance* GI = nullptr;
		if (GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.OwningGameInstance)
				{
					GI = Context.OwningGameInstance;
					break;
				}
			}
		}
		if (GI)
		{
			TestWorld->SetGameInstance(GI);
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		TestDrop = TestWorld->SpawnActor<ASBPhysicalLootDrop>(ASBPhysicalLootDrop::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestDrop->SetRole(ROLE_Authority);

		FSBGameplayTags::InitializeNativeTags();

		// Item sem fragmento de raridade (deve virar Comum)
		CommonItemDef = NewObject<USBItemDefinition>(TestWorld, TEXT("CommonItem"));
		const_cast<FText&>(CommonItemDef->DisplayName) = FText::FromString(TEXT("Ferro"));

		// Item Lendário
		LegendaryItemDef = NewObject<USBItemDefinition>(TestWorld, TEXT("LegendaryItem"));
		const_cast<FText&>(LegendaryItemDef->DisplayName) = FText::FromString(TEXT("Espada Suprema"));
		
		USBItemFragment_Rarity* RarityFrag = NewObject<USBItemFragment_Rarity>(LegendaryItemDef);
		RarityFrag->RarityTag = FSBGameplayTags::Get().Loot_Rarity_Legendary;
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(LegendaryItemDef->Fragments).Add(RarityFrag);
	});

	AfterEach([this]()
	{
		if (TestDrop)
		{
			TestDrop->Destroy();
			TestDrop = nullptr;
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("ShouldDefaultToCommonRarity", [this]()
	{
		TestDrop->InitializeLoot(CommonItemDef, 1);
		
		FGameplayTag ExpectedRarity = FSBGameplayTags::Get().Loot_Rarity_Common;
		TestEqual("Item comum deve ter tag de raridade Comum", TestDrop->GetRarityTag(), ExpectedRarity);

		FLinearColor ExpectedColor = FLinearColor(0.7f, 0.7f, 0.7f);
		TestEqual("Cor do item comum deve ser cinza padrão", TestDrop->GetRarityColor(), ExpectedColor);
	});

	It("ShouldRetrieveConfiguredRarityAndColor", [this]()
	{
		TestDrop->InitializeLoot(LegendaryItemDef, 1);

		FGameplayTag ExpectedRarity = FSBGameplayTags::Get().Loot_Rarity_Legendary;
		TestEqual("Item deve ter tag de raridade Lendária", TestDrop->GetRarityTag(), ExpectedRarity);

		FLinearColor ExpectedColor = FLinearColor(1.0f, 0.5f, 0.0f);
		TestEqual("Cor do item lendário deve ser ouro/laranja", TestDrop->GetRarityColor(), ExpectedColor);
	});

	It("ShouldMapAllRarityTagsToExpectedColors", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// Mock fragmento para teste inline
		USBItemFragment_Rarity* TestRarityFrag = NewObject<USBItemFragment_Rarity>(TestWorld);
		USBItemDefinition* TempItemDef = NewObject<USBItemDefinition>(TestWorld);
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(TempItemDef->Fragments).Add(TestRarityFrag);

		// Uncommon
		TestRarityFrag->RarityTag = Tags.Loot_Rarity_Uncommon;
		TestDrop->InitializeLoot(TempItemDef, 1);
		TestEqual("Uncommon deve ser verde", TestDrop->GetRarityColor(), FLinearColor(0.1f, 0.8f, 0.1f));

		// Rare
		TestRarityFrag->RarityTag = Tags.Loot_Rarity_Rare;
		TestDrop->InitializeLoot(TempItemDef, 1);
		TestEqual("Rare deve ser azul", TestDrop->GetRarityColor(), FLinearColor(0.1f, 0.4f, 0.9f));

		// Epic
		TestRarityFrag->RarityTag = Tags.Loot_Rarity_Epic;
		TestDrop->InitializeLoot(TempItemDef, 1);
		TestEqual("Epic deve ser roxo", TestDrop->GetRarityColor(), FLinearColor(0.6f, 0.1f, 0.8f));
	});

	It("ShouldNotCrashWhenUpdatingVisualsWithoutMaterials", [this]()
	{
		TestDrop->InitializeLoot(LegendaryItemDef, 1);
		
		// Executa UpdateVisuals sem material definido no MeshComponent (não deve quebrar)
		TestDrop->UpdateVisuals();
		
		TestTrue("Execução síncrona sem crash", true);
	});
}
