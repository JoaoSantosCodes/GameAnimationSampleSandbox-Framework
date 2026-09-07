// Copyright 2026 João Santos. All Rights Reserved.
// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBCraftingComponent.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemFragment_Salvageable.h"

BEGIN_DEFINE_SPEC(FSBSalvageTestsSpec, "Sandbox.Salvage", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBInventoryComponent* InventoryComponent;
	USBCraftingComponent* CraftingComponent;

	USBItemDefinition* ItemDefIronSword;
	USBItemDefinition* ItemDefIronOre;
	USBItemDefinition* ItemDefCoal;
	USBItemDefinition* ItemDefApple;

	USBItemFragment_Salvageable* SalvageFragment;
END_DEFINE_SPEC(FSBSalvageTestsSpec)

void FSBSalvageTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		InventoryComponent = NewObject<USBInventoryComponent>(TestCharacter);
		InventoryComponent->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(InventoryComponent);
		ISBComponentInterface::Execute_OnReady(InventoryComponent);

		CraftingComponent = NewObject<USBCraftingComponent>(TestCharacter);
		CraftingComponent->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(CraftingComponent);
		ISBComponentInterface::Execute_OnReady(CraftingComponent);

		// Criação dos itens
		ItemDefIronSword = NewObject<USBItemDefinition>();
		ItemDefIronSword->DisplayName = FText::FromString(TEXT("Espada de Ferro"));
		ItemDefIronSword->MaxStackCount = 5;

		ItemDefIronOre = NewObject<USBItemDefinition>();
		ItemDefIronOre->DisplayName = FText::FromString(TEXT("Minério de Ferro"));
		ItemDefIronOre->MaxStackCount = 100;

		ItemDefCoal = NewObject<USBItemDefinition>();
		ItemDefCoal->DisplayName = FText::FromString(TEXT("Carvão"));
		ItemDefCoal->MaxStackCount = 100;

		ItemDefApple = NewObject<USBItemDefinition>();
		ItemDefApple->DisplayName = FText::FromString(TEXT("Maçã"));
		ItemDefApple->MaxStackCount = 10;

		// Configuração do fragmento Salvageable para a Espada
		SalvageFragment = NewObject<USBItemFragment_Salvageable>(ItemDefIronSword);
		
		// Drop Garantido 1: Minério de ferro (1 a 2 unidades, 100% de chance)
		FSBSalvageOutcome GuaranteedOre;
		GuaranteedOre.ItemDef = ItemDefIronOre;
		GuaranteedOre.MinQuantity = 1;
		GuaranteedOre.MaxQuantity = 2;
		GuaranteedOre.Probability = 1.0f;
		SalvageFragment->PotentialOutcomes.Add(GuaranteedOre);

		// Drop Garantido 2: Carvão (exatamente 2 unidades, 100% de chance)
		FSBSalvageOutcome GuaranteedCoal;
		GuaranteedCoal.ItemDef = ItemDefCoal;
		GuaranteedCoal.MinQuantity = 2;
		GuaranteedCoal.MaxQuantity = 2;
		GuaranteedCoal.Probability = 1.0f;
		SalvageFragment->PotentialOutcomes.Add(GuaranteedCoal);

		// Drop Impossível: Minério de ferro extra (0% de chance)
		FSBSalvageOutcome ImpossibleDrop;
		ImpossibleDrop.ItemDef = ItemDefIronOre;
		ImpossibleDrop.MinQuantity = 10;
		ImpossibleDrop.MaxQuantity = 10;
		ImpossibleDrop.Probability = 0.0f;
		SalvageFragment->PotentialOutcomes.Add(ImpossibleDrop);

		ItemDefIronSword->Fragments.Add(SalvageFragment);
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

	It("Tentar desmantelar item sem fragmento salvageable deve falhar", [this]()
	{
		USBItemInstance* AppleInstance = InventoryComponent->ServerAddItem(ItemDefApple, 1);
		TestNotNull("ItemInstance de maçã criado", AppleInstance);

		bool bResult = CraftingComponent->ServerSalvageItem(AppleInstance, 1);
		TestFalse("Salvaging de maçã deve retornar falso", bResult);
		TestEqual("Inventário ainda possui a maçã", InventoryComponent->GetTotalItemQuantity(ItemDefApple), 1);
	});

	It("Desmantelar item com fragmento deve consumir item original e conceder drops garantidos", [this]()
	{
		USBItemInstance* SwordInstance = InventoryComponent->ServerAddItem(ItemDefIronSword, 1);
		TestNotNull("SwordInstance criado", SwordInstance);

		bool bResult = CraftingComponent->ServerSalvageItem(SwordInstance, 1);
		TestTrue("Salvaging deve retornar verdadeiro", bResult);

		// Espada consumida
		TestEqual("Espada foi removida do inventário", InventoryComponent->GetTotalItemQuantity(ItemDefIronSword), 0);

		// Drops garantidos recebidos
		int32 OreCount = InventoryComponent->GetTotalItemQuantity(ItemDefIronOre);
		TestTrue("Recebeu entre 1 e 2 minérios de ferro", OreCount >= 1 && OreCount <= 2);

		int32 CoalCount = InventoryComponent->GetTotalItemQuantity(ItemDefCoal);
		TestEqual("Recebeu exatamente 2 carvões", CoalCount, 2);
	});

	It("Desmantelar multiplas unidades simultaneas deve multiplicar os drops proporcionalmente", [this]()
	{
		USBItemInstance* SwordInstance = InventoryComponent->ServerAddItem(ItemDefIronSword, 3);
		TestNotNull("SwordInstance com stack de 3 criado", SwordInstance);

		bool bResult = CraftingComponent->ServerSalvageItem(SwordInstance, 3);
		TestTrue("Salvaging deve retornar verdadeiro", bResult);

		// Espadas consumidas
		TestEqual("Espadas foram removidas do inventário", InventoryComponent->GetTotalItemQuantity(ItemDefIronSword), 0);

		// Drops de Carvão multiplicados por 3 (2 por espada = 6 carvões)
		int32 CoalCount = InventoryComponent->GetTotalItemQuantity(ItemDefCoal);
		TestEqual("Recebeu exatamente 6 carvões (3 x 2)", CoalCount, 6);

		// Drops de Minério multiplicados por 3 (entre 3 x 1 e 3 x 2)
		int32 OreCount = InventoryComponent->GetTotalItemQuantity(ItemDefIronOre);
		TestTrue("Recebeu entre 3 e 6 minérios de ferro", OreCount >= 3 && OreCount <= 6);
	});
}
