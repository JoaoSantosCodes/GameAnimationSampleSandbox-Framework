#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBCraftingComponent.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment_Armor.h"
#include "Items/SBItemFragment_Durability.h"
#include "Items/SBItemFragment_Upgrade.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBUpgradeTestsSpec, "Sandbox.Inventory.Upgrade", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestCharacter;
	USBInventoryComponent* InventoryComp;
	USBAttributeComponent* AttrComp;
	USBStateComponent* StateComp;
	USBCraftingComponent* CraftingComp;

	USBItemDefinition* IronOreDef;
	USBItemDefinition* BasicArmorDef;
	USBItemDefinition* NonUpgradableDef;

	FGameplayTag DefenseTag;
	FGameplayTag CoinsTag;
	FGameplayTag ForgeStationTag;
END_DEFINE_SPEC(FSBUpgradeTestsSpec)

void FSBUpgradeTestsSpec::Define()
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

		StateComp = NewObject<USBStateComponent>(TestCharacter);
		StateComp->RegisterComponent();

		CraftingComp = NewObject<USBCraftingComponent>(TestCharacter);
		CraftingComp->RegisterComponent();

		FSBGameplayTags::InitializeNativeTags();
		DefenseTag = FSBGameplayTags::Get().Attribute_Defense;
		CoinsTag = FSBGameplayTags::Get().Attribute_Coins;
		ForgeStationTag = FGameplayTag::RequestGameplayTag(TEXT("Crafting.Station.Forge"), false);
		if (!ForgeStationTag.IsValid())
		{
			// Caso a tag não exista nativamente, criamos
			ForgeStationTag = FSBGameplayTags::Get().State_Character_Dead; // fallback seguro pra teste
		}

		// Registra atributos necessários
		FSBAttribute DefenseAttr;
		DefenseAttr.BaseValue = 0.0f;
		DefenseAttr.CurrentValue = 0.0f;
		DefenseAttr.MaxValue = 1000.0f;
		DefenseAttr.MinValue = 0.0f;
		AttrComp->RegisterAttribute(DefenseTag, DefenseAttr);

		FSBAttribute CoinsAttr;
		CoinsAttr.BaseValue = 0.0f;
		CoinsAttr.CurrentValue = 0.0f;
		CoinsAttr.MaxValue = 1000000.0f;
		CoinsAttr.MinValue = 0.0f;
		AttrComp->RegisterAttribute(CoinsTag, CoinsAttr);

		// Material: Iron Ore
		IronOreDef = NewObject<USBItemDefinition>(TestWorld, TEXT("IronOreDef"));
		const_cast<FText&>(IronOreDef->DisplayName) = FText::FromString(TEXT("Minério de Ferro"));

		// Item sem upgrade
		NonUpgradableDef = NewObject<USBItemDefinition>(TestWorld, TEXT("NonUpgradableDef"));

		// Equipamento Melhorável: Basic Armor (+10 de Defesa base)
		BasicArmorDef = NewObject<USBItemDefinition>(TestWorld, TEXT("BasicArmorDef"));
		const_cast<FText&>(BasicArmorDef->DisplayName) = FText::FromString(TEXT("Armadura de Ferro"));

		USBItemFragment_Armor* ArmorFrag = NewObject<USBItemFragment_Armor>(BasicArmorDef);
		ArmorFrag->EquipmentSlotTag = FSBGameplayTags::Get().State_Item_Equipped;
		FSBItemAttributeModifier DefenseMod;
		DefenseMod.AttributeTag = DefenseTag;
		DefenseMod.ModifierType = ESBAttributeModifierType::Additive;
		DefenseMod.Magnitude = 10.0f;
		ArmorFrag->ModifiersToGrant.Add(DefenseMod);
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(BasicArmorDef->Fragments).Add(ArmorFrag);

		USBItemFragment_Durability* DurabilityFrag = NewObject<USBItemFragment_Durability>(BasicArmorDef);
		DurabilityFrag->MaxDurability = 100.0f;
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(BasicArmorDef->Fragments).Add(DurabilityFrag);

		// Configura fragmento de upgrade: +1 (+10% stats, 2 Iron Ore, 50 coins) e +2 (+20% stats, 4 Iron Ore, 100 coins)
		USBItemFragment_Upgrade* UpgradeFrag = NewObject<USBItemFragment_Upgrade>(BasicArmorDef);
		
		FSBUpgradeCostPerLevel Cost1;
		FSBCraftingIngredient Ingr1;
		Ingr1.ItemDef = IronOreDef;
		Ingr1.Quantity = 2;
		Cost1.Ingredients.Add(Ingr1);
		Cost1.CoinCost = 50;
		Cost1.StatMultiplierBonus = 0.1f; // +10% (Multiplicador total 1.1)

		FSBUpgradeCostPerLevel Cost2;
		FSBCraftingIngredient Ingr2;
		Ingr2.ItemDef = IronOreDef;
		Ingr2.Quantity = 4;
		Cost2.Ingredients.Add(Ingr2);
		Cost2.CoinCost = 100;
		Cost2.StatMultiplierBonus = 0.2f; // +20% (Multiplicador total 1.3)

		UpgradeFrag->UpgradeLevels.Add(Cost1);
		UpgradeFrag->UpgradeLevels.Add(Cost2);
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(BasicArmorDef->Fragments).Add(UpgradeFrag);
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

	It("ShouldNotUpgradeIfNoUpgradeFragment", [this]()
	{
		USBItemInstance* Item = InventoryComp->ServerAddItem(NonUpgradableDef, 1);
		TestNotNull(TEXT("Item instanciado com sucesso"), Item);

		bool bSuccess = CraftingComp->ServerUpgradeItem(Item);
		TestFalse(TEXT("Item sem fragmento de upgrade não deve poder ser melhorado"), bSuccess);
		TestEqual(TEXT("Nível de upgrade deve permanecer 0"), Item->UpgradeLevel, 0);
	});

	It("ShouldNotUpgradeIfInsufficientMaterialsOrCoins", [this]()
	{
		USBItemInstance* Item = InventoryComp->ServerAddItem(BasicArmorDef, 1);
		
		// Sem ingredientes nem moedas
		bool bSuccess = CraftingComp->ServerUpgradeItem(Item);
		TestFalse(TEXT("Upgrade deve falhar sem insumos"), bSuccess);

		// Apenas moedas
		AttrComp->SetAttributeBaseValue(CoinsTag, 100.0f);
		bSuccess = CraftingComp->ServerUpgradeItem(Item);
		TestFalse(TEXT("Upgrade deve falhar sem materiais"), bSuccess);

		// Apenas materiais
		AttrComp->SetAttributeBaseValue(CoinsTag, 0.0f);
		InventoryComp->ServerAddItem(IronOreDef, 10);
		bSuccess = CraftingComp->ServerUpgradeItem(Item);
		TestFalse(TEXT("Upgrade deve falhar sem moedas suficientes"), bSuccess);
		TestEqual(TEXT("Nível de upgrade deve permanecer 0"), Item->UpgradeLevel, 0);
	});

	It("ShouldUpgradeItemConsumingIngredientsAndCoins", [this]()
	{
		USBItemInstance* Item = InventoryComp->ServerAddItem(BasicArmorDef, 1);
		
		// Fornece insumos
		InventoryComp->ServerAddItem(IronOreDef, 5);
		AttrComp->SetAttributeBaseValue(CoinsTag, 100.0f);

		bool bSuccess = CraftingComp->ServerUpgradeItem(Item);
		TestTrue(TEXT("Upgrade de nível 1 deve suceder"), bSuccess);
		TestEqual(TEXT("Nível de upgrade deve ir para 1 (+1)"), Item->UpgradeLevel, 1);

		// Verifica consumo
		TestEqual(TEXT("Minérios restantes devem ser 3 (5 - 2)"), InventoryComp->GetTotalItemQuantity(IronOreDef), 3);
		TestEqual(TEXT("Moedas restantes devem ser 50 (100 - 50)"), AttrComp->GetAttributeValue(CoinsTag), 50.0f);
	});

	It("ShouldScaleDefenseAttributeWhenEquippedAndUpgraded", [this]()
	{
		USBItemInstance* Item = InventoryComp->ServerAddItem(BasicArmorDef, 1);
		
		// Fornece insumos suficientes para dois upgrades
		InventoryComp->ServerAddItem(IronOreDef, 10);
		AttrComp->SetAttributeBaseValue(CoinsTag, 300.0f);

		// Equipar no nível +0
		InventoryComp->ServerEquipItem(Item);
		TestEqual(TEXT("Defesa no nível base deve ser +10"), AttrComp->GetAttributeValue(DefenseTag), 10.0f);

		// Upgrade 1 (+1): Multiplicador +10% (Total 1.1x) -> Defesa 11.0f
		bool bSuccess = CraftingComp->ServerUpgradeItem(Item);
		TestTrue(TEXT("Primeiro upgrade deve suceder"), bSuccess);
		TestEqual(TEXT("Nível deve ser +1"), Item->UpgradeLevel, 1);
		TestEqual(TEXT("Defesa após upgrade +1 deve ser +11 (10 * 1.1)"), AttrComp->GetAttributeValue(DefenseTag), 11.0f);

		// Upgrade 2 (+2): Multiplicador +20% adicional (Total 1.3x) -> Defesa 13.0f
		bSuccess = CraftingComp->ServerUpgradeItem(Item);
		TestTrue(TEXT("Segundo upgrade deve suceder"), bSuccess);
		TestEqual(TEXT("Nível deve ser +2"), Item->UpgradeLevel, 2);
		TestEqual(TEXT("Defesa após upgrade +2 deve ser +13 (10 * 1.3)"), AttrComp->GetAttributeValue(DefenseTag), 13.0f);
	});

	It("ShouldRestoreDurabilityOnUpgrade", [this]()
	{
		USBItemInstance* Item = InventoryComp->ServerAddItem(BasicArmorDef, 1);
		Item->SetDurability_Implementation(10.0f);
		TestEqual(TEXT("Durabilidade inicial danificada"), Item->Durability, 10.0f);

		InventoryComp->ServerAddItem(IronOreDef, 5);
		AttrComp->SetAttributeBaseValue(CoinsTag, 100.0f);

		bool bSuccess = CraftingComp->ServerUpgradeItem(Item);
		TestTrue(TEXT("Upgrade bem-sucedido"), bSuccess);
		TestEqual(TEXT("Durabilidade deve ter sido totalmente reparada para 100"), Item->Durability, 100.0f);
	});

	It("ShouldRequireStationTagIfConfigured", [this]()
	{
		// Configura estação necessária no fragmento
		USBItemFragment_Upgrade* UpgradeFrag = const_cast<USBItemFragment_Upgrade*>(Cast<USBItemFragment_Upgrade>(BasicArmorDef->FindFragmentByClass(USBItemFragment_Upgrade::StaticClass())));
		UpgradeFrag->RequiredStationTag = ForgeStationTag;

		USBItemInstance* Item = InventoryComp->ServerAddItem(BasicArmorDef, 1);
		InventoryComp->ServerAddItem(IronOreDef, 5);
		AttrComp->SetAttributeBaseValue(CoinsTag, 100.0f);

		// Sem estar perto da forja (falha)
		bool bSuccess = CraftingComp->ServerUpgradeItem(Item);
		TestFalse(TEXT("Deve falhar sem a estação requerida"), bSuccess);

		// Entra no estado da estação de forja
		StateComp->AddTag(ForgeStationTag);
		
		bSuccess = CraftingComp->ServerUpgradeItem(Item);
		TestTrue(TEXT("Deve suceder estando perto da estação requerida"), bSuccess);
	});
}
