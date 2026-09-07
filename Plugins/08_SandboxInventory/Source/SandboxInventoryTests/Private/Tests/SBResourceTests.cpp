// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/SBInventoryComponent.h"
#include "Actors/SBResourceNode.h"
#include "DataAssets/SBLootTableDataAsset.h"
#include "Items/SBItemDefinition.h"
#include "GameplayTagsManager.h"
#include "SBGameplayTags.h"
#include "Engine/DamageEvents.h"

BEGIN_DEFINE_SPEC(FSBResourceTestsSpec, "Sandbox.Inventory.Resource", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ACharacter* PlayerCharacter;
	USBInventoryComponent* InventoryComponent;
	USBItemDefinition* WoodItemDef;
	USBLootTableDataAsset* LootTableAsset;
	FGameplayTag PickaxeTag;
	FGameplayTag AxeTag;
END_DEFINE_SPEC(FSBResourceTestsSpec)

void FSBResourceTestsSpec::Define()
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

		FSBGameplayTags::InitializeNativeTags();

		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		PickaxeTag = TagsManager.AddNativeGameplayTag(TEXT("Tool.Pickaxe"));
		AxeTag = TagsManager.AddNativeGameplayTag(TEXT("Tool.Axe"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("ResourceTestPlayerPawn");
		PlayerCharacter = TestWorld->SpawnActor<ACharacter>(ACharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		InventoryComponent = NewObject<USBInventoryComponent>(PlayerCharacter, TEXT("InventoryComponent"));
		InventoryComponent->RegisterComponent();
		PlayerCharacter->DispatchBeginPlay();

		// Cria item de drop de teste
		WoodItemDef = NewObject<USBItemDefinition>(TestWorld, TEXT("WoodItemDef"));
		const_cast<FText&>(WoodItemDef->DisplayName) = FText::FromString(TEXT("Madeira"));
		const_cast<int32&>(WoodItemDef->MaxStackCount) = 99;

		// Cria loot table
		LootTableAsset = NewObject<USBLootTableDataAsset>(TestWorld, TEXT("TestLootTable"));
		FSBLootEntry Entry;
		Entry.ItemDefinition = WoodItemDef;
		Entry.MinStackCount = 5;
		Entry.MaxStackCount = 5;
		Entry.Weight = 1.0f;
		Entry.DropChance = 1.0f;
		LootTableAsset->Entries.Add(Entry);
	});

	AfterEach([this]()
	{
		if (PlayerCharacter)
		{
			PlayerCharacter->Destroy();
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
		}
	});

	It("Should apply full damage when using the correct tool", [this]()
	{
		FActorSpawnParameters NodeParams;
		NodeParams.Name = TEXT("GoldOreNode");
		ASBResourceNode* ResourceNode = TestWorld->SpawnActor<ASBResourceNode>(ASBResourceNode::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, NodeParams);
		ResourceNode->DispatchBeginPlay();
		
		// Configura o nó de minério exigindo picareta
		FProperty* RequiredToolProp = ASBResourceNode::StaticClass()->FindPropertyByName(TEXT("RequiredToolTag"));
		if (RequiredToolProp)
		{
			RequiredToolProp->ContainerPtrToValuePtr<FGameplayTag>(ResourceNode)[0] = PickaxeTag;
		}

		ResourceNode->SetDebugActiveToolTag(PickaxeTag);

		// Aplica dano
		float Applied = ResourceNode->TakeDamage(30.f, FDamageEvent(), nullptr, PlayerCharacter);

		TestEqual("Damage should be fully applied", Applied, 30.f);
		TestEqual("Resource health should decrease by 30", ResourceNode->GetHealth(), 70.f);

		ResourceNode->Destroy();
	});

	It("Should mitigate damage when using the incorrect tool or bare hands", [this]()
	{
		FActorSpawnParameters NodeParams;
		NodeParams.Name = TEXT("WoodNode");
		ASBResourceNode* ResourceNode = TestWorld->SpawnActor<ASBResourceNode>(ASBResourceNode::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, NodeParams);
		ResourceNode->DispatchBeginPlay();

		// Configura o nó exigindo picareta
		FProperty* RequiredToolProp = ASBResourceNode::StaticClass()->FindPropertyByName(TEXT("RequiredToolTag"));
		if (RequiredToolProp)
		{
			RequiredToolProp->ContainerPtrToValuePtr<FGameplayTag>(ResourceNode)[0] = PickaxeTag;
		}

		// Jogador usa machado (ferramenta incorreta)
		ResourceNode->SetDebugActiveToolTag(AxeTag);

		// Aplica dano
		float Applied = ResourceNode->TakeDamage(30.f, FDamageEvent(), nullptr, PlayerCharacter);

		// Dano esperado mitigado para 10%: 30 * 0.1 = 3
		TestEqual("Damage should be mitigated to 10%", Applied, 3.f);
		TestEqual("Resource health should decrease only by 3", ResourceNode->GetHealth(), 97.f);

		ResourceNode->Destroy();
	});

	It("Should deplete resource, grant loot, and trigger respawn", [this]()
	{
		FActorSpawnParameters NodeParams;
		NodeParams.Name = TEXT("IronOreNode");
		ASBResourceNode* ResourceNode = TestWorld->SpawnActor<ASBResourceNode>(ASBResourceNode::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, NodeParams);
		ResourceNode->DispatchBeginPlay();

		// Configura loot table
		FProperty* LootProp = ASBResourceNode::StaticClass()->FindPropertyByName(TEXT("LootTable"));
		if (LootProp)
		{
			LootProp->ContainerPtrToValuePtr<TObjectPtr<USBLootTableDataAsset>>(ResourceNode)[0] = LootTableAsset;
		}

		// Configura tempo de respawn baixo
		FProperty* RespawnProp = ASBResourceNode::StaticClass()->FindPropertyByName(TEXT("RespawnTime"));
		if (RespawnProp)
		{
			RespawnProp->ContainerPtrToValuePtr<float>(ResourceNode)[0] = 0.5f;
		}

		TestEqual("Respawn time should be 0.5f", ResourceNode->GetRespawnTime(), 0.5f);

		// Aplica dano fatal
		ResourceNode->TakeDamage(100.f, FDamageEvent(), nullptr, PlayerCharacter);

		TestTrue("Resource should be depleted", ResourceNode->IsDepleted());
		TestEqual("Resource health should be 0", ResourceNode->GetHealth(), 0.f);

		// Verifica se o loot foi concedido (5 unidades de madeira)
		TestEqual("Wood count in inventory should be 5", InventoryComponent->GetTotalItemQuantity(WoodItemDef), 5);

		// Verifica se o timer de respawn está ativo no gerenciador de timers
		TestTrue("Respawn timer should be active", TestWorld->GetTimerManager().IsTimerActive(ResourceNode->GetRespawnTimerHandle()));

		// Força o respawn direto chamando o método de debug
		ResourceNode->DebugForceRespawn();

		TestFalse("Resource should no longer be depleted after respawn", ResourceNode->IsDepleted());
		TestEqual("Resource health should be fully restored", ResourceNode->GetHealth(), 100.f);

		ResourceNode->Destroy();
	});
}
