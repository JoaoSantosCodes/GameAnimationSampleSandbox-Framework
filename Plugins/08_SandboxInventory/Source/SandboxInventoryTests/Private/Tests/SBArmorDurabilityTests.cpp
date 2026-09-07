// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBCraftingComponent.h"
#include "Components/SBStateComponent.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemFragment_Armor.h"
#include "Items/SBItemFragment_Durability.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBArmorDurabilityTestsSpec, "Sandbox.Inventory.ArmorDurability", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestCharacter;
	USBInventoryComponent* InventoryComp;
	USBAttributeComponent* AttrComp;
	USBCraftingComponent* CraftingComp;

	USBItemDefinition* ArmorItemDef;
	USBItemFragment_Durability* DurabilityFragment;
	USBItemFragment_Armor* ArmorFragment;

	FGameplayTag HealthTag;
	FGameplayTag DefenseTag;
	FGameplayTag ChestSlotTag;
END_DEFINE_SPEC(FSBArmorDurabilityTestsSpec)

void FSBArmorDurabilityTestsSpec::Define()
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
		HealthTag = FSBGameplayTags::Get().Attribute_Health;
		DefenseTag = FSBGameplayTags::Get().Attribute_Defense;
		ChestSlotTag = FSBGameplayTags::Get().Equipment_Slot_Chest;

		FSBAttribute HealthAttr;
		HealthAttr.BaseValue = 100.0f;
		HealthAttr.CurrentValue = 100.0f;
		HealthAttr.MaxValue = 100.0f;
		HealthAttr.MinValue = 0.0f;
		AttrComp->RegisterAttribute(HealthTag, HealthAttr);

		FSBAttribute DefenseAttr;
		DefenseAttr.BaseValue = 0.0f;
		DefenseAttr.CurrentValue = 0.0f;
		DefenseAttr.MaxValue = 100.0f;
		DefenseAttr.MinValue = 0.0f;
		AttrComp->RegisterAttribute(DefenseTag, DefenseAttr);

		// Inicializa o binding explicitamente simulando BeginPlay se necessário
		ISBComponentInterface::Execute_OnInitialize(InventoryComp);
		// OnPostInitialize e onde o inventario cacheia e assina o componente de atributos.
		// Pular este gancho deixava a durabilidade sem consumo.
		ISBComponentInterface::Execute_OnPostInitialize(InventoryComp);
		ISBComponentInterface::Execute_OnReady(InventoryComp);
		
		ISBComponentInterface::Execute_OnInitialize(AttrComp);
		ISBComponentInterface::Execute_OnReady(AttrComp);

		ISBComponentInterface::Execute_OnInitialize(CraftingComp);
		ISBComponentInterface::Execute_OnReady(CraftingComp);

		// Cria definição de armadura com durabilidade e atributos
		ArmorItemDef = NewObject<USBItemDefinition>(TestWorld, TEXT("ArmorItemDef"));
		const_cast<int32&>(ArmorItemDef->MaxStackCount) = 1;

		DurabilityFragment = NewObject<USBItemFragment_Durability>(ArmorItemDef);
		DurabilityFragment->MaxDurability = 100.0f;
		DurabilityFragment->InitialDurability = 100.0f;
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(ArmorItemDef->Fragments).Add(DurabilityFragment);

		ArmorFragment = NewObject<USBItemFragment_Armor>(ArmorItemDef);
		ArmorFragment->EquipmentSlotTag = ChestSlotTag;
		FSBItemAttributeModifier Mod;
		Mod.AttributeTag = DefenseTag;
		Mod.ModifierType = ESBAttributeModifierType::Additive;
		Mod.Magnitude = 20.0f;
		ArmorFragment->ModifiersToGrant.Add(Mod);
		const_cast<TArray<TObjectPtr<USBItemFragment>>&>(ArmorItemDef->Fragments).Add(ArmorFragment);
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

	It("ShouldReduceDurabilityOnDamage", [this]()
	{
		USBItemInstance* Item = InventoryComp->ServerAddItem(ArmorItemDef, 1);
		InventoryComp->ServerEquipItem(Item);

		TestEqual(TEXT("Durabilidade deve iniciar cheia"), Item->Durability, 100.0f);

		// Aplica dano ao jogador (redução de Health)
		AttrComp->SetAttributeBaseValue(HealthTag, 85.0f); // 15 de dano

		TestEqual(TEXT("Durabilidade deve ter caido proporcionalmente pelo dano de 15"), Item->Durability, 85.0f);
	});

	It("ShouldDeactivateModifiersWhenDurabilityHitsZero", [this]()
	{
		USBItemInstance* Item = InventoryComp->ServerAddItem(ArmorItemDef, 1);
		InventoryComp->ServerEquipItem(Item);

		TestEqual(TEXT("Defesa com armadura equipada deve ser 20"), AttrComp->GetAttributeValue(DefenseTag), 20.0f);

		// Aplica dano massivo de 105 para quebrar a armadura
		AttrComp->SetAttributeBaseValue(HealthTag, 0.0f); 

		TestEqual(TEXT("Durabilidade deve ter ido a 0"), Item->Durability, 0.0f);
		TestEqual(TEXT("Modificadores de defesa devem ter sido desativados (Defesa vai a 0)"), AttrComp->GetAttributeValue(DefenseTag), 0.0f);
	});

	It("ShouldNotApplyModifiersOnEquipIfAlreadyBroken", [this]()
	{
		USBItemInstance* Item = InventoryComp->ServerAddItem(ArmorItemDef, 1);
		Item->SetDurability_Implementation(0.0f); // Quebra antes de equipar

		InventoryComp->ServerEquipItem(Item);

		TestTrue(TEXT("Ainda deve constar equipado visualmente/logicamente"), Item->DynamicTags.HasTagExact(FSBGameplayTags::Get().State_Item_Equipped));
		TestEqual(TEXT("Não deve ter ativado a defesa por estar quebrada"), AttrComp->GetAttributeValue(DefenseTag), 0.0f);
	});

	It("ShouldReapplyModifiersAfterRepair", [this]()
	{
		USBItemInstance* Item = InventoryComp->ServerAddItem(ArmorItemDef, 1);
		InventoryComp->ServerEquipItem(Item);

		// Quebra a armadura
		AttrComp->SetAttributeBaseValue(HealthTag, 0.0f);
		TestEqual(TEXT("Defesa deve ser 0 (quebrada)"), AttrComp->GetAttributeValue(DefenseTag), 0.0f);

		// Entra no estado de proximidade a uma bancada
		USBStateComponent* StateComp = TestCharacter->FindComponentByClass<USBStateComponent>();
		if (!StateComp)
		{
			StateComp = NewObject<USBStateComponent>(TestCharacter);
			StateComp->RegisterComponent();
			ISBComponentInterface::Execute_OnInitialize(StateComp);
			ISBComponentInterface::Execute_OnReady(StateComp);
		}
		StateComp->AddTag(FSBGameplayTags::Get().Crafting_Station_Forge);

		// Executa reparo
		bool bRepaired = CraftingComp->ServerRepairItem(Item);
		TestTrue(TEXT("Reparo deve suceder"), bRepaired);
		TestEqual(TEXT("Durabilidade restaurada ao maximo (100)"), Item->Durability, 100.0f);

		// Os modificadores devem ter sido ativados novamente!
		TestEqual(TEXT("Defesa deve ser restaurada para 20 após reparo"), AttrComp->GetAttributeValue(DefenseTag), 20.0f);
	});
}
