// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Actors/SBContainerChest.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Character/SBCharacter.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment_Durability.h"
#include "Interfaces/SBItemDurabilityInterface.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBContainerTestsSpec, "Sandbox.Inventory.Chest", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBContainerChest* TestChest;
	ASBCharacter* TestCharacter;
	USBInventoryComponent* PlayerInv;
	USBStateComponent* PlayerState;
	USBAttributeComponent* PlayerAttr;
	USBItemDefinition* ResourceItemDef;
	USBItemDefinition* DurableItemDef;
END_DEFINE_SPEC(FSBContainerTestsSpec)

void FSBContainerTestsSpec::Define()
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
		
		// Spawn do baú
		TestChest = TestWorld->SpawnActor<ASBContainerChest>(ASBContainerChest::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestChest->SetRole(ROLE_Authority);
		
		// Spawn do jogador
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		FSBGameplayTags::InitializeNativeTags();

		// Inicializa componentes do jogador
		PlayerState = NewObject<USBStateComponent>(TestCharacter, TEXT("StateComponent"));
		PlayerState->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(PlayerState);
		ISBComponentInterface::Execute_OnReady(PlayerState);

		PlayerAttr = NewObject<USBAttributeComponent>(TestCharacter, TEXT("AttributeComponent"));
		PlayerAttr->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(PlayerAttr);
		ISBComponentInterface::Execute_OnReady(PlayerAttr);

		PlayerInv = NewObject<USBInventoryComponent>(TestCharacter, TEXT("InventoryComponent"));
		PlayerInv->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(PlayerInv);
		ISBComponentInterface::Execute_OnReady(PlayerInv);

		// Inicializa o baú
		ISBComponentInterface::Execute_OnInitialize(TestChest->InventoryComponent);
		ISBComponentInterface::Execute_OnReady(TestChest->InventoryComponent);

		// Cria item de recurso simples
		ResourceItemDef = NewObject<USBItemDefinition>(TestWorld, TEXT("ResourceItemDef"));
		const_cast<FText&>(ResourceItemDef->DisplayName) = FText::FromString(TEXT("Madeira"));
		const_cast<int32&>(ResourceItemDef->MaxStackCount) = 100;

		// Cria item durável
		DurableItemDef = NewObject<USBItemDefinition>(TestWorld, TEXT("DurableItemDef"));
		const_cast<FText&>(DurableItemDef->DisplayName) = FText::FromString(TEXT("Machado"));
		const_cast<int32&>(DurableItemDef->MaxStackCount) = 1;

		USBItemFragment_Durability* DurabilityFrag = NewObject<USBItemFragment_Durability>(DurableItemDef);
		DurabilityFrag->MaxDurability = 100.0f;
		DurabilityFrag->InitialDurability = 100.0f;
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(DurableItemDef->Fragments).Add(DurabilityFrag);
	});

	AfterEach([this]()
	{
		if (TestCharacter)
		{
			TestCharacter->Destroy();
			TestCharacter = nullptr;
		}
		if (TestChest)
		{
			TestChest->Destroy();
			TestChest = nullptr;
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("ShouldOpenAndTrackInteractors", [this]()
	{
		FGameplayTag InteractingTag = FSBGameplayTags::Get().State_Character_Interacting;

		// 1. Jogador interage com o baú
		TestChest->Interact_Implementation(TestCharacter);

		TestTrue("Jogador deve ganhar tag de interagindo", PlayerState->HasTag(InteractingTag));

		// 2. Jogador se afasta (simula movimentação a 500 unidades e tick)
		TestCharacter->SetActorLocation(FVector(500.0f, 0.0f, 0.0f));
		TestChest->Tick(0.1f);

		TestFalse("Jogador deve perder tag ao se afastar demais", PlayerState->HasTag(InteractingTag));
	});

	It("ShouldTransferItemsBetweenPlayerAndChest", [this]()
	{
		// Adiciona 10 madeiras no inventário do jogador
		USBItemInstance* PlayerItem = PlayerInv->ServerAddItem(ResourceItemDef, 10);
		TestNotNull("Deve criar item no inventário do jogador", PlayerItem);

		// Transfere 4 madeiras para o baú
		bool bTransfer = PlayerInv->ServerTransferItem(TestChest->InventoryComponent, PlayerItem, 4);
		TestTrue("Transferência deve ser bem-sucedida", bTransfer);

		TestEqual("Jogador deve ficar com 6 madeiras", PlayerInv->GetTotalItemQuantity(ResourceItemDef), 6);
		TestEqual("Baú deve ficar com 4 madeiras", TestChest->InventoryComponent->GetTotalItemQuantity(ResourceItemDef), 4);

		// Transfere 2 madeiras de volta do baú para o jogador
		TArray<USBItemInstance*> ChestItems = TestChest->InventoryComponent->GetAllItems();
		TestTrue("Baú deve conter itens", ChestItems.Num() > 0);
		if (ChestItems.Num() > 0)
		{
			bool bTransferBack = TestChest->InventoryComponent->ServerTransferItem(PlayerInv, ChestItems[0], 2);
			TestTrue("Transferência de volta deve funcionar", bTransferBack);
		}

		TestEqual("Jogador deve ficar com 8 madeiras", PlayerInv->GetTotalItemQuantity(ResourceItemDef), 8);
		TestEqual("Baú deve ficar com 2 madeiras", TestChest->InventoryComponent->GetTotalItemQuantity(ResourceItemDef), 2);
	});

	It("ShouldBlockTransferIfTooFar", [this]()
	{
		// Posiciona o jogador a 1000 unidades de distância (limite é 600)
		TestCharacter->SetActorLocation(FVector(1000.0f, 0.0f, 0.0f));

		USBItemInstance* PlayerItem = PlayerInv->ServerAddItem(ResourceItemDef, 10);

		// Tenta transferir
		bool bTransfer = PlayerInv->ServerTransferItem(TestChest->InventoryComponent, PlayerItem, 4);
		TestFalse("Deve bloquear transferência por distância excessiva", bTransfer);

		TestEqual("Jogador deve manter as 10 madeiras", PlayerInv->GetTotalItemQuantity(ResourceItemDef), 10);
		TestEqual("Baú não deve receber nada", TestChest->InventoryComponent->GetTotalItemQuantity(ResourceItemDef), 0);
	});

	It("ShouldPreserveMetadataOnTransfer", [this]()
	{
		// Adiciona 1 machado ao jogador
		USBItemInstance* Machado = PlayerInv->ServerAddItem(DurableItemDef, 1);
		TestNotNull("Machado deve ser criado", Machado);
		
		// Danifica o machado
		ISBItemDurabilityInterface::Execute_SetDurability(Machado, 42.0f);

		// Transfere o machado para o baú
		bool bTransfer = PlayerInv->ServerTransferItem(TestChest->InventoryComponent, Machado, 1);
		TestTrue("Machado deve ser transferido", bTransfer);

		// Verifica se o machado no baú preservou a durabilidade de 42.f
		TArray<USBItemInstance*> ChestItems = TestChest->InventoryComponent->GetAllItems();
		TestTrue("Baú deve conter o machado", ChestItems.Num() > 0);
		if (ChestItems.Num() > 0)
		{
			float ChestItemDurability = ISBItemDurabilityInterface::Execute_GetDurability(ChestItems[0]);
			TestEqual("Durabilidade no baú deve ser 42", ChestItemDurability, 42.0f);
		}
	});
}
