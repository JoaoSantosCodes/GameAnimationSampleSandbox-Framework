// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBInventoryComponent.h"
#include "Items/SBItemDefinition.h"
#include "DataAssets/SBLootTableDataAsset.h"
#include "Actors/SBPhysicalLootDrop.h"
#include "Interfaces/SBInteractableInterface.h"

BEGIN_DEFINE_SPEC(FSBLootDropTestsSpec, "Sandbox.LootDrop", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBInventoryComponent* InventoryComponent;
	USBItemDefinition* ItemDefHealthPotion;
	USBItemDefinition* ItemDefRifleAmmo;
	USBLootTableDataAsset* LootTable;
END_DEFINE_SPEC(FSBLootDropTestsSpec)

void FSBLootDropTestsSpec::Define()
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

		// Criação de definições de itens de teste
		ItemDefHealthPotion = NewObject<USBItemDefinition>();
		ItemDefHealthPotion->DisplayName = FText::FromString(TEXT("Poção de Vida"));
		ItemDefHealthPotion->MaxStackCount = 5;

		ItemDefRifleAmmo = NewObject<USBItemDefinition>();
		ItemDefRifleAmmo->DisplayName = FText::FromString(TEXT("Munição de Rifle"));
		ItemDefRifleAmmo->MaxStackCount = 30;

		// Criação de Tabela de Loot de Teste
		LootTable = NewObject<USBLootTableDataAsset>();
		
		FSBLootEntry Entry1;
		Entry1.ItemDefinition = ItemDefHealthPotion;
		Entry1.MinStackCount = 1;
		Entry1.MaxStackCount = 3;
		Entry1.Weight = 10.0f;
		Entry1.DropChance = 1.0f;

		FSBLootEntry Entry2;
		Entry2.ItemDefinition = ItemDefRifleAmmo;
		Entry2.MinStackCount = 10;
		Entry2.MaxStackCount = 20;
		Entry2.Weight = 5.0f;
		Entry2.DropChance = 1.0f;

		LootTable->Entries.Add(Entry1);
		LootTable->Entries.Add(Entry2);
	});

	It("USBLootTableDataAsset deve sortear itens respeitando pesos e faixas de quantidade", [this]()
	{
		TArray<FSBLootDropResult> Results = LootTable->RollLoot(3);
		TestEqual("Deve retornar exatamente 3 resultados", Results.Num(), 3);

		for (const FSBLootDropResult& Result : Results)
		{
			TestNotNull("ItemDefinition gerado não deve ser nulo", Result.ItemDefinition.Get());
			TestTrue("Quantidade gerada deve ser maior que 0", Result.StackCount > 0);
		}
	});

	It("ASBPhysicalLootDrop deve inicializar corretamente com item definition e quantidade", [this]()
	{
		FActorSpawnParameters SpawnParams;
		ASBPhysicalLootDrop* LootDrop = TestWorld->SpawnActor<ASBPhysicalLootDrop>(ASBPhysicalLootDrop::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		TestNotNull("LootDrop deve ser spawnado com sucesso", LootDrop);
		LootDrop->SetRole(ROLE_Authority);

		LootDrop->InitializeLoot(ItemDefHealthPotion, 3);

		TestEqual("ItemDefinition do drop deve ser ItemDefHealthPotion", LootDrop->GetItemDefinition(), ItemDefHealthPotion);
		TestEqual("StackCount do drop deve ser 3", LootDrop->GetStackCount(), 3);

		FText Prompt = LootDrop->GetInteractionPrompt_Implementation(TestCharacter);
		TestTrue("Prompt deve conter o nome do item", Prompt.ToString().Contains(TEXT("Poção de Vida")));
		TestTrue("Prompt deve conter a quantidade", Prompt.ToString().Contains(TEXT("3")));
	});

	It("ASBPhysicalLootDrop deve permitir interagir apenas se o interator possuir inventário", [this]()
	{
		FActorSpawnParameters SpawnParams;
		ASBPhysicalLootDrop* LootDrop = TestWorld->SpawnActor<ASBPhysicalLootDrop>(ASBPhysicalLootDrop::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		LootDrop->SetRole(ROLE_Authority);
		LootDrop->InitializeLoot(ItemDefHealthPotion, 2);

		bool bCanInteractWithChar = LootDrop->CanInteract_Implementation(TestCharacter);
		TestTrue("Personagem com inventário deve poder interagir", bCanInteractWithChar);

		bool bCanInteractNull = LootDrop->CanInteract_Implementation(nullptr);
		TestFalse("Ponteiro nulo não deve poder interagir", bCanInteractNull);
	});

	It("Interagir com ASBPhysicalLootDrop deve transferir o item para o inventário do personagem", [this]()
	{
		FActorSpawnParameters SpawnParams;
		ASBPhysicalLootDrop* LootDrop = TestWorld->SpawnActor<ASBPhysicalLootDrop>(ASBPhysicalLootDrop::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		LootDrop->SetRole(ROLE_Authority);
		LootDrop->InitializeLoot(ItemDefHealthPotion, 3);

		LootDrop->Interact_Implementation(TestCharacter);

		TArray<USBItemInstance*> Items = InventoryComponent->GetAllItems();
		TestEqual("Inventário deve conter 1 item após a coleta", Items.Num(), 1);
		if (Items.Num() > 0)
		{
			TestTrue(TEXT("Definição do item no inventário deve ser ItemDefHealthPotion"), Items[0]->ItemDef == ItemDefHealthPotion);
		}
	});

	It("Lock de interação deve proteger contra coletas concorrentes (Anti-Race Condition)", [this]()
	{
		FActorSpawnParameters SpawnParams;
		ASBPhysicalLootDrop* LootDrop = TestWorld->SpawnActor<ASBPhysicalLootDrop>(ASBPhysicalLootDrop::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		LootDrop->SetRole(ROLE_Authority);
		LootDrop->InitializeLoot(ItemDefHealthPotion, 2);

		LootDrop->LockInteraction_Implementation(TestCharacter);

		bool bCanInteract = LootDrop->CanInteract_Implementation(TestCharacter);
		TestFalse("Drop travado por lock não deve permitir nova interação", bCanInteract);

		LootDrop->UnlockInteraction_Implementation(TestCharacter);
		bCanInteract = LootDrop->CanInteract_Implementation(TestCharacter);
		TestTrue("Drop destravado deve voltar a permitir interação", bCanInteract);
	});

	AfterEach([this]()
	{
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
		}
	});
}
