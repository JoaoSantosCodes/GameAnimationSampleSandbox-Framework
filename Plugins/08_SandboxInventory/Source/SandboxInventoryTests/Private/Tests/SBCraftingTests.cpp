// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBCraftingComponent.h"
#include "DataAssets/SBCraftingRecipeDataAsset.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemInstance.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBCraftingTestsSpec, "Sandbox.Inventory.Crafting", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBInventoryComponent* InventoryComponent;
	USBStateComponent* StateComponent;
	USBCraftingComponent* CraftingComponent;

	USBItemDefinition* ItemDefWood;
	USBItemDefinition* ItemDefPlank;
	USBItemDefinition* ItemDefIronIngot;
	USBItemDefinition* ItemDefIronSword;
	USBItemDefinition* ItemDefHerb;
	USBItemDefinition* ItemDefWater;
	USBItemDefinition* ItemDefPotion;

	USBCraftingRecipeDataAsset* RecipePlanks;
	USBCraftingRecipeDataAsset* RecipeIronSword;
	USBCraftingRecipeDataAsset* RecipePotion;
END_DEFINE_SPEC(FSBCraftingTestsSpec)

void FSBCraftingTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		FSBGameplayTags::InitializeNativeTags();

		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnReady(StateComponent);

		InventoryComponent = NewObject<USBInventoryComponent>(TestCharacter);
		InventoryComponent->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(InventoryComponent);
		ISBComponentInterface::Execute_OnReady(InventoryComponent);

		CraftingComponent = NewObject<USBCraftingComponent>(TestCharacter);
		CraftingComponent->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(CraftingComponent);
		ISBComponentInterface::Execute_OnReady(CraftingComponent);

		// Itens de Teste
		ItemDefWood = NewObject<USBItemDefinition>();
		ItemDefWood->DisplayName = FText::FromString(TEXT("Madeira"));
		ItemDefWood->MaxStackCount = 99;

		ItemDefPlank = NewObject<USBItemDefinition>();
		ItemDefPlank->DisplayName = FText::FromString(TEXT("Tábua de Madeira"));
		ItemDefPlank->MaxStackCount = 99;

		ItemDefIronIngot = NewObject<USBItemDefinition>();
		ItemDefIronIngot->DisplayName = FText::FromString(TEXT("Barra de Ferro"));
		ItemDefIronIngot->MaxStackCount = 99;

		ItemDefIronSword = NewObject<USBItemDefinition>();
		ItemDefIronSword->DisplayName = FText::FromString(TEXT("Espada de Ferro"));
		ItemDefIronSword->MaxStackCount = 1;

		ItemDefHerb = NewObject<USBItemDefinition>();
		ItemDefHerb->DisplayName = FText::FromString(TEXT("Erva Medicinal"));
		ItemDefHerb->MaxStackCount = 99;

		ItemDefWater = NewObject<USBItemDefinition>();
		ItemDefWater->DisplayName = FText::FromString(TEXT("Frasco de Água"));
		ItemDefWater->MaxStackCount = 99;

		ItemDefPotion = NewObject<USBItemDefinition>();
		ItemDefPotion->DisplayName = FText::FromString(TEXT("Poção de Vida"));
		ItemDefPotion->MaxStackCount = 20;

		// 1. Receita Básica: 2 Madeira -> 4 Tábuas (sem estação)
		RecipePlanks = NewObject<USBCraftingRecipeDataAsset>();
		RecipePlanks->DisplayName = FText::FromString(TEXT("Fabricar Tábuas"));
		FSBCraftingIngredient WoodIng;
		WoodIng.ItemDef = ItemDefWood;
		WoodIng.Quantity = 2;
		RecipePlanks->Ingredients.Add(WoodIng);
		RecipePlanks->ResultItemDef = ItemDefPlank;
		RecipePlanks->ResultQuantity = 4;

		// 2. Receita com Estação: 2 Barra de Ferro + Forja -> 1 Espada de Ferro
		RecipeIronSword = NewObject<USBCraftingRecipeDataAsset>();
		RecipeIronSword->DisplayName = FText::FromString(TEXT("Forjar Espada de Ferro"));
		RecipeIronSword->RequiredStationTag = FSBGameplayTags::Get().Crafting_Station_Forge;
		FSBCraftingIngredient IronIng;
		IronIng.ItemDef = ItemDefIronIngot;
		IronIng.Quantity = 2;
		RecipeIronSword->Ingredients.Add(IronIng);
		RecipeIronSword->ResultItemDef = ItemDefIronSword;
		RecipeIronSword->ResultQuantity = 1;

		// 3. Receita Multi-ingrediente: 2 Erva + 1 Frasco de Água -> 2 Poções
		RecipePotion = NewObject<USBCraftingRecipeDataAsset>();
		RecipePotion->DisplayName = FText::FromString(TEXT("Criar Poções de Cura"));
		FSBCraftingIngredient HerbIng;
		HerbIng.ItemDef = ItemDefHerb;
		HerbIng.Quantity = 2;
		RecipePotion->Ingredients.Add(HerbIng);

		FSBCraftingIngredient WaterIng;
		WaterIng.ItemDef = ItemDefWater;
		WaterIng.Quantity = 1;
		RecipePotion->Ingredients.Add(WaterIng);

		RecipePotion->ResultItemDef = ItemDefPotion;
		RecipePotion->ResultQuantity = 2;
	});

	AfterEach([this]()
	{
		if (TestCharacter)
		{
			TestCharacter->Destroy();
			TestCharacter = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(true);
			TestWorld = nullptr;
		}
	});

	It("Should craft recipe successfully when materials are sufficient and grant result item", [this]()
	{
		InventoryComponent->ServerAddItem(ItemDefWood, 5);
		TestEqual("Madeira inicial deve ser 5", InventoryComponent->GetTotalItemQuantity(ItemDefWood), 5);

		TestTrue("CanCraftRecipe deve retornar true para tábuas", CraftingComponent->CanCraftRecipe(RecipePlanks));

		bool bCrafted = CraftingComponent->ServerCraftRecipe(RecipePlanks);
		TestTrue("ServerCraftRecipe deve retornar true", bCrafted);

		// Consumiu 2 madeiras (5 - 2 = 3) e gerou 4 tábuas
		TestEqual("Madeira restante deve ser 3", InventoryComponent->GetTotalItemQuantity(ItemDefWood), 3);
		TestEqual("Tábuas criadas devem ser 4", InventoryComponent->GetTotalItemQuantity(ItemDefPlank), 4);
	});

	It("Should fail craft when ingredients are missing or insufficient and preserve inventory", [this]()
	{
		InventoryComponent->ServerAddItem(ItemDefWood, 1);
		TestEqual("Madeira inicial deve ser 1 (insuficiente)", InventoryComponent->GetTotalItemQuantity(ItemDefWood), 1);

		TestFalse("CanCraftRecipe deve retornar false", CraftingComponent->CanCraftRecipe(RecipePlanks));

		bool bCrafted = CraftingComponent->ServerCraftRecipe(RecipePlanks);
		TestFalse("ServerCraftRecipe deve falhar", bCrafted);

		TestEqual("Madeira deve permanecer intacta em 1", InventoryComponent->GetTotalItemQuantity(ItemDefWood), 1);
		TestEqual("Nenhuma tábua deve ter sido criada", InventoryComponent->GetTotalItemQuantity(ItemDefPlank), 0);
	});

	It("Should block craft when required crafting station tag is missing", [this]()
	{
		InventoryComponent->ServerAddItem(ItemDefIronIngot, 5);

		// Personagem NÃO possui a tag Crafting.Station.Forge
		TestFalse("CanCraftRecipe deve falhar por falta de forja", CraftingComponent->CanCraftRecipe(RecipeIronSword));

		bool bCrafted = CraftingComponent->ServerCraftRecipe(RecipeIronSword);
		TestFalse("ServerCraftRecipe deve ser bloqueado", bCrafted);

		TestEqual("Ferro deve continuar em 5", InventoryComponent->GetTotalItemQuantity(ItemDefIronIngot), 5);
		TestEqual("Nenhuma espada deve ter sido forjada", InventoryComponent->GetTotalItemQuantity(ItemDefIronSword), 0);
	});

	It("Should succeed craft when required crafting station tag is present in state component", [this]()
	{
		InventoryComponent->ServerAddItem(ItemDefIronIngot, 5);

		// Concede a tag de estação de forja no personagem
		StateComponent->AddTag(FSBGameplayTags::Get().Crafting_Station_Forge);

		TestTrue("CanCraftRecipe deve permitir com a forja ativa", CraftingComponent->CanCraftRecipe(RecipeIronSword));

		bool bCrafted = CraftingComponent->ServerCraftRecipe(RecipeIronSword);
		TestTrue("ServerCraftRecipe deve forjar a espada", bCrafted);

		TestEqual("Ferro restante deve ser 3", InventoryComponent->GetTotalItemQuantity(ItemDefIronIngot), 3);
		TestEqual("Espada criada deve ser 1", InventoryComponent->GetTotalItemQuantity(ItemDefIronSword), 1);
	});

	It("Should support multi-ingredient crafting with custom result quantities", [this]()
	{
		InventoryComponent->ServerAddItem(ItemDefHerb, 4);
		InventoryComponent->ServerAddItem(ItemDefWater, 2);

		TestTrue("CanCraftRecipe deve validar ambos os ingredientes", CraftingComponent->CanCraftRecipe(RecipePotion));

		bool bCrafted = CraftingComponent->ServerCraftRecipe(RecipePotion);
		TestTrue("ServerCraftRecipe deve concluir", bCrafted);

		// 4 Ervas - 2 = 2; 2 Águas - 1 = 1; 2 Poções criadas
		TestEqual("Ervas restantes devem ser 2", InventoryComponent->GetTotalItemQuantity(ItemDefHerb), 2);
		TestEqual("Frascos de água restantes devem ser 1", InventoryComponent->GetTotalItemQuantity(ItemDefWater), 1);
		TestEqual("Poções de vida criadas devem ser 2", InventoryComponent->GetTotalItemQuantity(ItemDefPotion), 2);
	});
}
