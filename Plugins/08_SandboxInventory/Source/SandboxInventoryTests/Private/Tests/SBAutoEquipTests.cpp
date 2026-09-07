// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBCraftingComponent.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment_Armor.h"
#include "Items/SBItemFragment_Upgrade.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBAutoEquipTestsSpec, "Sandbox.Inventory.AutoEquip", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestCharacter;
	USBInventoryComponent* InventoryComp;
	USBAttributeComponent* AttrComp;
	USBCraftingComponent* CraftingComp;

	USBItemDefinition* BasicHelmetDef;
	USBItemDefinition* SuperiorHelmetDef;
	USBItemDefinition* UpgradableHelmetDef;

	FGameplayTag DefenseTag;
	FGameplayTag HeadSlotTag;
END_DEFINE_SPEC(FSBAutoEquipTestsSpec)

void FSBAutoEquipTestsSpec::Define()
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
		TestCharacter = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		// Um AActor puro nao tem RootComponent: sem ele, SetActorLocation, SetActorTransform
		// e TeleportTo falham em silencio e o ator fica preso na origem.
		USceneComponent* TestCharacterRoot = NewObject<USceneComponent>(TestCharacter, TEXT("TestCharacterRoot"));
		TestCharacter->SetRootComponent(TestCharacterRoot);
		TestCharacterRoot->RegisterComponent();
		TestCharacter->SetRole(ROLE_Authority);

		InventoryComp = NewObject<USBInventoryComponent>(TestCharacter);
		InventoryComp->RegisterComponent();

		AttrComp = NewObject<USBAttributeComponent>(TestCharacter);
		AttrComp->RegisterComponent();

		CraftingComp = NewObject<USBCraftingComponent>(TestCharacter);
		CraftingComp->RegisterComponent();

		FSBGameplayTags::InitializeNativeTags();
		DefenseTag = FSBGameplayTags::Get().Attribute_Defense;
		HeadSlotTag = FSBGameplayTags::Get().Equipment_Slot_Head;

		FSBAttribute DefenseAttr;
		DefenseAttr.BaseValue = 0.0f;
		DefenseAttr.CurrentValue = 0.0f;
		DefenseAttr.MaxValue = 1000.0f;
		DefenseAttr.MinValue = 0.0f;
		AttrComp->RegisterAttribute(DefenseTag, DefenseAttr);

		// Capacete Básico: +5 Defesa
		BasicHelmetDef = NewObject<USBItemDefinition>(TestWorld, TEXT("BasicHelmetDef"));
		USBItemFragment_Armor* BasicFrag = NewObject<USBItemFragment_Armor>(BasicHelmetDef);
		BasicFrag->EquipmentSlotTag = HeadSlotTag;
		FSBItemAttributeModifier Mod1;
		Mod1.AttributeTag = DefenseTag;
		Mod1.ModifierType = ESBAttributeModifierType::Additive;
		Mod1.Magnitude = 5.0f;
		BasicFrag->ModifiersToGrant.Add(Mod1);
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(BasicHelmetDef->Fragments).Add(BasicFrag);

		// Capacete Superior: +10 Defesa
		SuperiorHelmetDef = NewObject<USBItemDefinition>(TestWorld, TEXT("SuperiorHelmetDef"));
		USBItemFragment_Armor* SuperiorFrag = NewObject<USBItemFragment_Armor>(SuperiorHelmetDef);
		SuperiorFrag->EquipmentSlotTag = HeadSlotTag;
		FSBItemAttributeModifier Mod2;
		Mod2.AttributeTag = DefenseTag;
		Mod2.ModifierType = ESBAttributeModifierType::Additive;
		Mod2.Magnitude = 10.0f;
		SuperiorFrag->ModifiersToGrant.Add(Mod2);
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(SuperiorHelmetDef->Fragments).Add(SuperiorFrag);

		// Capacete Melhorável: +4 Defesa base, com upgrade +1 (+50% bônus) -> vira +6
		UpgradableHelmetDef = NewObject<USBItemDefinition>(TestWorld, TEXT("UpgradableHelmetDef"));
		USBItemFragment_Armor* UpgradableFrag = NewObject<USBItemFragment_Armor>(UpgradableHelmetDef);
		UpgradableFrag->EquipmentSlotTag = HeadSlotTag;
		FSBItemAttributeModifier Mod3;
		Mod3.AttributeTag = DefenseTag;
		Mod3.ModifierType = ESBAttributeModifierType::Additive;
		Mod3.Magnitude = 4.0f;
		UpgradableFrag->ModifiersToGrant.Add(Mod3);
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(UpgradableHelmetDef->Fragments).Add(UpgradableFrag);

		USBItemFragment_Upgrade* UpgradeFrag = NewObject<USBItemFragment_Upgrade>(UpgradableHelmetDef);
		FSBUpgradeCostPerLevel Cost1;
		Cost1.CoinCost = 0;
		Cost1.StatMultiplierBonus = 0.5f; // +50%
		UpgradeFrag->UpgradeLevels.Add(Cost1);
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(UpgradableHelmetDef->Fragments).Add(UpgradeFrag);
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

	It("ShouldCalculateEffectiveDefenseCorrectly", [this]()
	{
		USBItemInstance* BasicItem = InventoryComp->ServerAddItem(BasicHelmetDef, 1);
		USBItemInstance* UpgradableItem = InventoryComp->ServerAddItem(UpgradableHelmetDef, 1);

		TestEqual(TEXT("Defesa efetiva básica deve ser 5"), InventoryComp->CalculateEffectiveDefense(BasicItem), 5.0f);
		TestEqual(TEXT("Defesa efetiva melhorável +0 deve ser 4"), InventoryComp->CalculateEffectiveDefense(UpgradableItem), 4.0f);

		// Executa upgrade
		bool bUpgradeSuccess = CraftingComp->ServerUpgradeItem(UpgradableItem);
		TestTrue(TEXT("Upgrade do capacete deve suceder"), bUpgradeSuccess);
		TestEqual(TEXT("Nível de upgrade deve ser +1"), UpgradableItem->UpgradeLevel, 1);
		TestEqual(TEXT("Defesa efetiva melhorável +1 deve ser 6 (4 * 1.5)"), InventoryComp->CalculateEffectiveDefense(UpgradableItem), 6.0f);
	});

	It("ShouldAutoEquipBetterArmorWhenTriggeredManually", [this]()
	{
		USBItemInstance* BasicItem = InventoryComp->ServerAddItem(BasicHelmetDef, 1);
		USBItemInstance* SuperiorItem = InventoryComp->ServerAddItem(SuperiorHelmetDef, 1);

		// Começa equipando o capacete básico
		InventoryComp->ServerEquipItem(BasicItem);
		TestTrue(TEXT("Capacete básico deve estar equipado"), BasicItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestEqual(TEXT("Defesa atual deve ser 5"), AttrComp->GetAttributeValue(DefenseTag), 5.0f);

		// Roda o auto-equipamento manualmente para cabeça
		bool bSwapped = InventoryComp->ServerAutoEquipBestArmor(HeadSlotTag);
		TestTrue(TEXT("Troca de armadura deve ter ocorrido"), bSwapped);

		// Verifica que o superior foi equipado e o básico desequipado
		TestTrue(TEXT("Capacete superior deve estar equipado"), SuperiorItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestFalse(TEXT("Capacete básico deve estar desequipado"), BasicItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestEqual(TEXT("Defesa deve subir para 10"), AttrComp->GetAttributeValue(DefenseTag), 10.0f);
	});

	It("ShouldAutoEquipOnLootAddedIfFlagEnabled", [this]()
	{
		// Ativa flag de auto-equipamento
		InventoryComp->bAutoEquipBetterLoot = true;

		// Adiciona o capacete básico (deve equipar sozinho por ser o melhor existente)
		USBItemInstance* BasicItem = InventoryComp->ServerAddItem(BasicHelmetDef, 1);
		TestTrue(TEXT("Capacete básico deve ter sido auto-equipado ao coletar"), BasicItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestEqual(TEXT("Defesa deve ser 5"), AttrComp->GetAttributeValue(DefenseTag), 5.0f);

		// Adiciona o capacete superior (deve substituir sozinho o básico)
		USBItemInstance* SuperiorItem = InventoryComp->ServerAddItem(SuperiorHelmetDef, 1);
		TestTrue(TEXT("Capacete superior deve ter sido auto-equipado substituindo o anterior"), SuperiorItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestFalse(TEXT("Capacete básico deve ter sido desequipado"), BasicItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestEqual(TEXT("Defesa deve ser 10"), AttrComp->GetAttributeValue(DefenseTag), 10.0f);
	});

	It("ShouldNotAutoEquipSuperiorArmorIfFlagDisabled", [this]()
	{
		// Mantém flag de auto-equipamento desativado
		InventoryComp->bAutoEquipBetterLoot = false;

		// Adiciona básico
		USBItemInstance* BasicItem = InventoryComp->ServerAddItem(BasicHelmetDef, 1);
		TestFalse(TEXT("Não deve equipar automaticamente"), BasicItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));

		// Equipamento manual
		InventoryComp->ServerEquipItem(BasicItem);
		TestTrue(TEXT("Equipado manualmente"), BasicItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));

		// Adiciona superior superior
		USBItemInstance* SuperiorItem = InventoryComp->ServerAddItem(SuperiorHelmetDef, 1);
		TestFalse(TEXT("Superior não deve ser equipado automaticamente"), SuperiorItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestTrue(TEXT("Básico deve permanecer equipado"), BasicItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestEqual(TEXT("Defesa deve permanecer 5"), AttrComp->GetAttributeValue(DefenseTag), 5.0f);
	});

	It("ShouldRespectUpgradeLevelsInComparison", [this]()
	{
		USBItemInstance* BasicItem = InventoryComp->ServerAddItem(BasicHelmetDef, 1); // 5.0 Defesa
		USBItemInstance* UpgradableItem = InventoryComp->ServerAddItem(UpgradableHelmetDef, 1); // 4.0 Defesa

		// Equipamos o básico inicialmente
		InventoryComp->ServerEquipItem(BasicItem);
		
		// Executa upgrade no outro item que vira 6.0 Defesa
		CraftingComp->ServerUpgradeItem(UpgradableItem);

		// Tenta auto-equipar
		bool bSwapped = InventoryComp->ServerAutoEquipBestArmor(HeadSlotTag);
		TestTrue(TEXT("Troca deve ocorrer devido à defesa escalada por upgrade"), bSwapped);

		TestTrue(TEXT("Melhorável +1 deve estar equipado"), UpgradableItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestFalse(TEXT("Básico deve ter sido desequipado"), BasicItem->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestEqual(TEXT("Defesa deve ser 6"), AttrComp->GetAttributeValue(DefenseTag), 6.0f);
	});
}
