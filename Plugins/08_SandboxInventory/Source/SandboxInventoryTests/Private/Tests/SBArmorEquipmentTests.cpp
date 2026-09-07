// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemFragment_Armor.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBArmorEquipmentTestsSpec, "Sandbox.Inventory.ArmorEquipment", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBInventoryComponent* InventoryComponent;
	USBAttributeComponent* AttributeComponent;

	USBItemDefinition* ItemDefHelmet;
	USBItemDefinition* ItemDefChestBasic;
	USBItemDefinition* ItemDefChestLegendary;
	USBItemDefinition* ItemDefBoots;
END_DEFINE_SPEC(FSBArmorEquipmentTestsSpec)

void FSBArmorEquipmentTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		FSBGameplayTags::InitializeNativeTags();

		AttributeComponent = NewObject<USBAttributeComponent>(TestCharacter);
		AttributeComponent->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(AttributeComponent);
		ISBComponentInterface::Execute_OnReady(AttributeComponent);

		InventoryComponent = NewObject<USBInventoryComponent>(TestCharacter);
		InventoryComponent->RegisterComponent();
		ISBComponentInterface::Execute_OnInitialize(InventoryComponent);
		ISBComponentInterface::Execute_OnReady(InventoryComponent);

		// Registra atributos de teste
		FSBAttribute DefenseAttr;
		DefenseAttr.BaseValue = 0.0f;
		DefenseAttr.CurrentValue = 0.0f;
		DefenseAttr.MaxValue = 1000.0f;
		DefenseAttr.MinValue = 0.0f;
		AttributeComponent->RegisterAttribute(FSBGameplayTags::Get().Attribute_Defense, DefenseAttr);

		FSBAttribute SpeedAttr;
		SpeedAttr.BaseValue = 600.0f;
		SpeedAttr.CurrentValue = 600.0f;
		SpeedAttr.MaxValue = 1500.0f;
		SpeedAttr.MinValue = 0.0f;
		AttributeComponent->RegisterAttribute(FSBGameplayTags::Get().Attribute_Speed, SpeedAttr);

		FSBAttribute StaminaAttr;
		StaminaAttr.BaseValue = 100.0f;
		StaminaAttr.CurrentValue = 100.0f;
		StaminaAttr.MaxValue = 200.0f;
		StaminaAttr.MinValue = 0.0f;
		AttributeComponent->RegisterAttribute(FSBGameplayTags::Get().Attribute_Stamina, StaminaAttr);

		// 1. Definição: Capacete (+25 Defesa)
		ItemDefHelmet = NewObject<USBItemDefinition>();
		ItemDefHelmet->DisplayName = FText::FromString(TEXT("Capacete de Ferro"));
		USBItemFragment_Armor* HelmetFrag = NewObject<USBItemFragment_Armor>(ItemDefHelmet);
		HelmetFrag->EquipmentSlotTag = FSBGameplayTags::Get().Equipment_Slot_Head;
		FSBItemAttributeModifier HelmetMod;
		HelmetMod.AttributeTag = FSBGameplayTags::Get().Attribute_Defense;
		HelmetMod.ModifierType = ESBAttributeModifierType::Additive;
		HelmetMod.Magnitude = 25.0f;
		HelmetFrag->ModifiersToGrant.Add(HelmetMod);
		ItemDefHelmet->Fragments.Add(HelmetFrag);

		// 2. Definição: Peitoral Básico (+30 Defesa)
		ItemDefChestBasic = NewObject<USBItemDefinition>();
		ItemDefChestBasic->DisplayName = FText::FromString(TEXT("Peitoral de Couro"));
		USBItemFragment_Armor* ChestBasicFrag = NewObject<USBItemFragment_Armor>(ItemDefChestBasic);
		ChestBasicFrag->EquipmentSlotTag = FSBGameplayTags::Get().Equipment_Slot_Chest;
		FSBItemAttributeModifier ChestBasicMod;
		ChestBasicMod.AttributeTag = FSBGameplayTags::Get().Attribute_Defense;
		ChestBasicMod.ModifierType = ESBAttributeModifierType::Additive;
		ChestBasicMod.Magnitude = 30.0f;
		ChestBasicFrag->ModifiersToGrant.Add(ChestBasicMod);
		ItemDefChestBasic->Fragments.Add(ChestBasicFrag);

		// 3. Definição: Peitoral Lendário (+80 Defesa)
		ItemDefChestLegendary = NewObject<USBItemDefinition>();
		ItemDefChestLegendary->DisplayName = FText::FromString(TEXT("Peitoral de Placas"));
		USBItemFragment_Armor* ChestLegendaryFrag = NewObject<USBItemFragment_Armor>(ItemDefChestLegendary);
		ChestLegendaryFrag->EquipmentSlotTag = FSBGameplayTags::Get().Equipment_Slot_Chest;
		FSBItemAttributeModifier ChestLegMod;
		ChestLegMod.AttributeTag = FSBGameplayTags::Get().Attribute_Defense;
		ChestLegMod.ModifierType = ESBAttributeModifierType::Additive;
		ChestLegMod.Magnitude = 80.0f;
		ChestLegendaryFrag->ModifiersToGrant.Add(ChestLegMod);
		ItemDefChestLegendary->Fragments.Add(ChestLegendaryFrag);

		// 4. Definição: Botas (+15 Defesa, +50 Velocidade, +20 Estamina)
		ItemDefBoots = NewObject<USBItemDefinition>();
		ItemDefBoots->DisplayName = FText::FromString(TEXT("Botas de Corrida"));
		USBItemFragment_Armor* BootsFrag = NewObject<USBItemFragment_Armor>(ItemDefBoots);
		BootsFrag->EquipmentSlotTag = FSBGameplayTags::Get().Equipment_Slot_Feet;

		FSBItemAttributeModifier BootsDefenseMod;
		BootsDefenseMod.AttributeTag = FSBGameplayTags::Get().Attribute_Defense;
		BootsDefenseMod.ModifierType = ESBAttributeModifierType::Additive;
		BootsDefenseMod.Magnitude = 15.0f;
		BootsFrag->ModifiersToGrant.Add(BootsDefenseMod);

		FSBItemAttributeModifier BootsSpeedMod;
		BootsSpeedMod.AttributeTag = FSBGameplayTags::Get().Attribute_Speed;
		BootsSpeedMod.ModifierType = ESBAttributeModifierType::Additive;
		BootsSpeedMod.Magnitude = 50.0f;
		BootsFrag->ModifiersToGrant.Add(BootsSpeedMod);

		FSBItemAttributeModifier BootsStaminaMod;
		BootsStaminaMod.AttributeTag = FSBGameplayTags::Get().Attribute_Stamina;
		BootsStaminaMod.ModifierType = ESBAttributeModifierType::Additive;
		BootsStaminaMod.Magnitude = 20.0f;
		BootsFrag->ModifiersToGrant.Add(BootsStaminaMod);

		ItemDefBoots->Fragments.Add(BootsFrag);
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

	It("Should grant attribute modifiers when armor item is equipped and calculate new attribute value", [this]()
	{
		USBItemInstance* HelmetInstance = InventoryComponent->ServerAddItem(ItemDefHelmet, 1);
		TestNotNull("Instância do capacete deve ser criada", HelmetInstance);

		TestEqual("Defesa inicial deve ser 0", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Defense), 0.0f);

		InventoryComponent->ServerEquipItem(HelmetInstance);

		TestTrue("Item deve receber tag de equipado", HelmetInstance->DynamicTags.HasTag(FSBGameplayTags::Get().State_Item_Equipped));
		TestTrue("Item deve receber tag de slot de cabeça", HelmetInstance->DynamicTags.HasTag(FSBGameplayTags::Get().Equipment_Slot_Head));
		TestEqual("Defesa deve ser incrementada para 25", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Defense), 25.0f);
	});

	It("Should remove attribute modifiers when armor item is unequipped", [this]()
	{
		USBItemInstance* HelmetInstance = InventoryComponent->ServerAddItem(ItemDefHelmet, 1);
		InventoryComponent->ServerEquipItem(HelmetInstance);
		TestEqual("Defesa equipada deve ser 25", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Defense), 25.0f);

		InventoryComponent->ServerUnequipItem(HelmetInstance);

		TestFalse("Item não deve mais ter tag de equipado", HelmetInstance->DynamicTags.HasTag(FSBGameplayTags::Get().State_Item_Equipped));
		TestFalse("Item não deve mais ter tag de slot", HelmetInstance->DynamicTags.HasTag(FSBGameplayTags::Get().Equipment_Slot_Head));
		TestEqual("Defesa deve voltar para 0", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Defense), 0.0f);
	});

	It("Should stack modifiers additively from multiple armor pieces in different slots", [this]()
	{
		USBItemInstance* HelmetInstance = InventoryComponent->ServerAddItem(ItemDefHelmet, 1);
		USBItemInstance* ChestInstance = InventoryComponent->ServerAddItem(ItemDefChestBasic, 1);

		InventoryComponent->ServerEquipItem(HelmetInstance);
		InventoryComponent->ServerEquipItem(ChestInstance);

		// 25 (Helmet) + 30 (Chest) = 55
		TestEqual("Defesa combinada de Capacete e Peitoral deve ser 55", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Defense), 55.0f);
	});

	It("Should automatically eject previous item when equipping new item in the same slot", [this]()
	{
		USBItemInstance* BasicChestInstance = InventoryComponent->ServerAddItem(ItemDefChestBasic, 1);
		USBItemInstance* LegendaryChestInstance = InventoryComponent->ServerAddItem(ItemDefChestLegendary, 1);

		// 1. Equipa Peitoral Básico (+30)
		InventoryComponent->ServerEquipItem(BasicChestInstance);
		TestTrue("Peitoral básico deve estar equipado", BasicChestInstance->DynamicTags.HasTag(FSBGameplayTags::Get().State_Item_Equipped));
		TestEqual("Defesa com peitoral básico deve ser 30", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Defense), 30.0f);

		// 2. Equipa Peitoral Lendário (+80) no mesmo slot (Equipment.Slot.Chest) -> Ejeção do básico
		InventoryComponent->ServerEquipItem(LegendaryChestInstance);

		TestFalse("Peitoral básico deve ter sido ejetado/desequipado", BasicChestInstance->DynamicTags.HasTag(FSBGameplayTags::Get().State_Item_Equipped));
		TestTrue("Peitoral lendário deve estar equipado", LegendaryChestInstance->DynamicTags.HasTag(FSBGameplayTags::Get().State_Item_Equipped));
		TestEqual("Defesa deve ser atualizada para 80 sem duplicação", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Defense), 80.0f);
	});

	It("Should support combined multi-attribute modifiers on a single armor piece (Defense, Speed, Stamina)", [this]()
	{
		USBItemInstance* BootsInstance = InventoryComponent->ServerAddItem(ItemDefBoots, 1);

		TestEqual("Velocidade inicial deve ser 600", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Speed), 600.0f);
		TestEqual("Estamina inicial deve ser 100", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Stamina), 100.0f);

		InventoryComponent->ServerEquipItem(BootsInstance);

		TestEqual("Defesa com botas deve ser 15", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Defense), 15.0f);
		TestEqual("Velocidade com botas deve ser 650", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Speed), 650.0f);
		TestEqual("Estamina com botas deve ser 120", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Stamina), 120.0f);

		InventoryComponent->ServerUnequipItem(BootsInstance);

		TestEqual("Defesa pós-desequipar deve voltar para 0", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Defense), 0.0f);
		TestEqual("Velocidade pós-desequipar deve voltar para 600", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Speed), 600.0f);
		TestEqual("Estamina pós-desequipar deve voltar para 100", AttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Stamina), 100.0f);
	});
}
