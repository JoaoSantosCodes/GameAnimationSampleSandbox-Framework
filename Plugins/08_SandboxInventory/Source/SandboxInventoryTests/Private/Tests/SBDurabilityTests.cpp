// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBCraftingComponent.h"
#include "Components/SBStateComponent.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment_Durability.h"
#include "Interfaces/SBItemDurabilityInterface.h"
#include "GameplayTagsManager.h"

BEGIN_DEFINE_SPEC(FSBDurabilityTestsSpec, "Sandbox.Inventory.Durability", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ACharacter* TestCharacter;
	USBInventoryComponent* InventoryComp;
	USBCraftingComponent* CraftingComp;
	USBStateComponent* StateComp;
	USBItemDefinition* ToolItemDef;
	USBItemFragment_Durability* DurabilityFragment;
END_DEFINE_SPEC(FSBDurabilityTestsSpec)

void FSBDurabilityTestsSpec::Define()
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
		SpawnParams.Name = TEXT("DurabilityTestPlayer");
		TestCharacter = TestWorld->SpawnActor<ACharacter>(ACharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		InventoryComp = NewObject<USBInventoryComponent>(TestCharacter, TEXT("InventoryComponent"));
		InventoryComp->RegisterComponent();

		CraftingComp = NewObject<USBCraftingComponent>(TestCharacter, TEXT("CraftingComponent"));
		CraftingComp->RegisterComponent();

		StateComp = NewObject<USBStateComponent>(TestCharacter, TEXT("StateComponent"));
		StateComp->RegisterComponent();

		// Inicializa Componentes
		ISBComponentInterface::Execute_OnInitialize(InventoryComp);
		ISBComponentInterface::Execute_OnPostInitialize(InventoryComp);
		ISBComponentInterface::Execute_OnReady(InventoryComp);
		ISBComponentInterface::Execute_OnInitialize(CraftingComp);
		ISBComponentInterface::Execute_OnReady(CraftingComp);
		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnReady(StateComp);

		// Cria definição de item com fragmento de durabilidade
		ToolItemDef = NewObject<USBItemDefinition>(TestWorld, TEXT("ToolItemDef"));
		const_cast<FText&>(ToolItemDef->DisplayName) = FText::FromString(TEXT("Picareta de Teste"));
		const_cast<int32&>(ToolItemDef->MaxStackCount) = 1;

		DurabilityFragment = NewObject<USBItemFragment_Durability>(ToolItemDef);
		DurabilityFragment->MaxDurability = 100.0f;
		DurabilityFragment->InitialDurability = 80.0f;
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(ToolItemDef->Fragments).Add(DurabilityFragment);
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
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("ShouldInitializeDurabilityFromFragment", [this]()
	{
		// 1. Adiciona o item ao inventário
		USBItemInstance* ItemInstance = InventoryComp->ServerAddItem(ToolItemDef, 1);
		TestNotNull("Instância do item não deve ser nula", ItemInstance);
		if (!ItemInstance) return;

		// 2. Verifica se a durabilidade foi inicializada com o valor do fragmento (80.f)
		float CurrentDurability = ISBItemDurabilityInterface::Execute_GetDurability(ItemInstance);
		TestEqual("A durabilidade inicial deve ser 80", CurrentDurability, 80.0f);
	});

	It("ShouldConsumeDurabilityOnUseAndReplicate", [this]()
	{
		USBItemInstance* ItemInstance = InventoryComp->ServerAddItem(ToolItemDef, 1);
		TestNotNull("Instância do item não deve ser nula", ItemInstance);
		if (!ItemInstance) return;

		// 1. Consome durabilidade
		ISBItemDurabilityInterface::Execute_ConsumeDurability(ItemInstance, 15.0f);
		TestEqual("Durabilidade local deve cair para 65", ItemInstance->Durability, 65.0f);

		// 2. Notifica e atualiza no inventário para simular replicação/atualização da struct
		InventoryComp->MarkItemInstanceUpdated(ItemInstance);

		// Busca na lista do inventário e valida sincronização da struct
		float EntryDurability = 0.0f;
		bool bFound = false;
		for (const FSBInventoryEntry& Entry : InventoryComp->GetInventoryList().Entries)
		{
			if (Entry.Instance == ItemInstance)
			{
				EntryDurability = Entry.Durability;
				bFound = true;
				break;
			}
		}

		TestTrue("Item encontrado na lista de entradas", bFound);
		TestEqual("Durabilidade na struct de replicação deve refletir 65", EntryDurability, 65.0f);
	});

	It("ShouldRepairItemAtCraftingStation", [this]()
	{
		USBItemInstance* ItemInstance = InventoryComp->ServerAddItem(ToolItemDef, 1);
		TestNotNull("Instância do item não deve ser nula", ItemInstance);
		if (!ItemInstance) return;

		// 1. Reduz durabilidade para 20.f
		ItemInstance->SetDurability_Implementation(20.0f);
		TestEqual("Durabilidade ajustada para 20", ItemInstance->Durability, 20.0f);

		// 2. Tenta reparar sem estar em uma bancada (deve falhar e manter 20.f)
		bool bFailedRepair = CraftingComp->ServerRepairItem(ItemInstance);
		TestFalse("Reparo deve falhar sem tag de bancada", bFailedRepair);
		TestEqual("Durabilidade deve continuar 20", ItemInstance->Durability, 20.0f);

		// 3. Adiciona tag de bancada e tenta novamente (deve funcionar e restaurar para MaxDurability de 100.f)
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		FGameplayTag AnvilTag = TagsManager.AddNativeGameplayTag(TEXT("Crafting.Station.Anvil"));
		StateComp->AddTag(AnvilTag);

		bool bSuccessRepair = CraftingComp->ServerRepairItem(ItemInstance);
		TestTrue("Reparo deve suceder com tag de bancada ativa", bSuccessRepair);
		TestEqual("Durabilidade deve ser restaurada para MaxDurability (100)", ItemInstance->Durability, 100.0f);
	});
}
