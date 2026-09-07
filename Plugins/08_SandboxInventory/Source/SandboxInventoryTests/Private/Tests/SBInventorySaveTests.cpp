#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SBSaveSubsystem.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBStateComponent.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemInstance.h"
#include "Character/SBCharacter.h"
#include "GameplayTagsManager.h"
#include "SBInventoryTestTypes.h"
#include "Items/SBItemFragment_Equippable.h"
#include "Subsystems/SBEventSubsystem.h"

BEGIN_DEFINE_SPEC(FSBInventorySaveTestsSpec, "Sandbox.Inventory.SaveSystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	UGameInstance* GameInstance;
	ASBCharacter* TestCharacter;
	USBInventoryComponent* InventoryComponent;
	USBStateComponent* StateComponent;

	FGameplayTag BrokenTag;
END_DEFINE_SPEC(FSBInventorySaveTestsSpec)

void FSBInventorySaveTestsSpec::Define()
{
	BeforeEach([this]()
	{
		GameInstance = NewObject<UGameInstance>(GEngine);
		GameInstance->InitializeStandalone();

		FWorldContext& NewContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		NewContext.OwningGameInstance = GameInstance;

		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		NewContext.SetCurrentWorld(TestWorld);

		TestWorld->SetGameInstance(GameInstance);

		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		TagsManager.AddNativeGameplayTag(TEXT("State.Item.Broken"));
		BrokenTag = FGameplayTag::RequestGameplayTag(TEXT("State.Item.Broken"));

		TagsManager.AddNativeGameplayTag(TEXT("State.Item.Equipped"));
		TagsManager.AddNativeGameplayTag(TEXT("Event.Inventory.ItemEquipped"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("PersistentCharacter");
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		StateComponent = NewObject<USBStateComponent>(TestCharacter, TEXT("PersistentState"));
		StateComponent->RegisterComponent();

		InventoryComponent = NewObject<USBInventoryComponent>(TestCharacter, TEXT("PersistentInventory"));
		InventoryComponent->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(StateComponent);
		ISBComponentInterface::Execute_OnInitialize(InventoryComponent);

		ISBComponentInterface::Execute_OnReady(StateComponent);
		ISBComponentInterface::Execute_OnReady(InventoryComponent);
	});

	AfterEach([this]()
	{
		if (TestCharacter)
		{
			TestWorld->DestroyActor(TestCharacter);
			TestCharacter = nullptr;
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
		GEngine->DestroyWorldContext(TestWorld);
	});

	It("Cenário 1: Salvar e Carregar slots de inventário e tags dinâmicas", [this]()
	{
		// 1. Cria definição de item transiente
		USBItemDefinition* ItemDef = NewObject<USBItemDefinition>(GetTransientPackage(), TEXT("TestItemDef"));
		ItemDef->DisplayName = FText::FromString(TEXT("Test Item"));
		ItemDef->MaxStackCount = 10;

		// 2. Adiciona ao inventário
		USBItemInstance* ItemInst = InventoryComponent->ServerAddItem(ItemDef, 5);
		TestNotNull("Item deve ser adicionado", ItemInst);
		ItemInst->DynamicTags.AddTag(BrokenTag);

		// 3. Salva
		USBSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USBSaveSubsystem>();
		TestNotNull("SaveSubsystem deve estar ativo", SaveSubsystem);

		bool bSaveSuccess = SaveSubsystem->SaveGame(TEXT("TestInventorySaveSlot"), 0);
		TestTrue("Salvamento deve ser bem-sucedido", bSaveSuccess);

		// Limpa da memória
		InventoryComponent->ServerRemoveItem(ItemInst, 5);
		TestEqual("Inventário deve estar vazio", InventoryComponent->GetAllItems().Num(), 0);

		// 4. Destrói e recria o personagem
		TestCharacter->Rename(TEXT("OldCharacterForDestruction"));
		InventoryComponent->Rename(TEXT("OldInventory"));
		TestWorld->DestroyActor(TestCharacter);
		TestCharacter = nullptr;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("PersistentCharacter");
		ASBCharacter* NewCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter = NewCharacter;

		USBStateComponent* NewState = NewObject<USBStateComponent>(NewCharacter, TEXT("PersistentState"));
		NewState->RegisterComponent();
		USBInventoryComponent* NewInventory = NewObject<USBInventoryComponent>(NewCharacter, TEXT("PersistentInventory"));
		NewInventory->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(NewState);
		ISBComponentInterface::Execute_OnInitialize(NewInventory);

		ISBComponentInterface::Execute_OnReady(NewState);
		ISBComponentInterface::Execute_OnReady(NewInventory);

		// 5. Carrega
		bool bLoadSuccess = SaveSubsystem->LoadGame(TEXT("TestInventorySaveSlot"), 0);
		TestTrue("Carregamento deve ser bem-sucedido", bLoadSuccess);

		// 6. Verifica itens restaurados
		TArray<USBItemInstance*> LoadedItems = NewInventory->GetAllItems();
		TestEqual("Deve haver exatamente 1 item no inventário", LoadedItems.Num(), 1);
		if (LoadedItems.Num() > 0)
		{
			TestEqual("StackCount do item carregado deve ser 5", LoadedItems[0]->StackCount, 5);
			TestTrue("Item carregado deve possuir tag quebrada", LoadedItems[0]->DynamicTags.HasTag(BrokenTag));
		}
	});

	It("Cenário 2: Persistência e restauração do estado equipado (Visual/Behavior)", [this]()
	{
		// 1. Cria definição de item equipável
		USBItemDefinition* WeaponItemDef = NewObject<USBItemDefinition>(GetTransientPackage(), TEXT("TestWeaponDef"));
		WeaponItemDef->DisplayName = FText::FromString(TEXT("Test Rifle"));
		WeaponItemDef->MaxStackCount = 1;

		USBItemFragment_Equippable* EquipFragment = NewObject<USBItemFragment_Equippable>(WeaponItemDef);
		WeaponItemDef->Fragments.Add(EquipFragment);

		// 2. Registra escutador de eventos
		USBTestInventoryListener* EventListener = NewObject<USBTestInventoryListener>();
		if (USBEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<USBEventSubsystem>())
		{
			EventSubsystem->SubscribeToEventNative(
				FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.ItemEquipped")),
				ESBEventPriority::Medium,
				FSBNativeEventDelegate::CreateUObject(EventListener, &USBTestInventoryListener::OnEquipped)
			);
		}

		// 3. Adiciona ao inventário e equipa
		USBItemInstance* ItemInst = InventoryComponent->ServerAddItem(WeaponItemDef, 1);
		TestNotNull("Item deve ser adicionado", ItemInst);
		InventoryComponent->ServerEquipItem(ItemInst);
		TestEqual("EquipCount inicial deve ser 1", EventListener->EquipCount, 1);

		FGameplayTag EquippedTag = FGameplayTag::RequestGameplayTag(TEXT("State.Item.Equipped"));
		TestTrue("Item deve possuir tag de equipado", ItemInst->DynamicTags.HasTag(EquippedTag));

		// 4. Salva o jogo
		USBSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<USBSaveSubsystem>();
		TestNotNull("SaveSubsystem deve estar ativo", SaveSubsystem);

		bool bSaveSuccess = SaveSubsystem->SaveGame(TEXT("TestWeaponSaveSlot"), 0);
		TestTrue("Salvamento deve ser bem-sucedido", bSaveSuccess);

		// 5. Destrói e recria o personagem simulando carregamento
		TestCharacter->Rename(TEXT("OldCharacterForDestruction2"));
		InventoryComponent->Rename(TEXT("OldInventory2"));
		TestWorld->DestroyActor(TestCharacter);
		TestCharacter = nullptr;

		// Reseta contadores do listener
		EventListener->EquipCount = 0;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("PersistentCharacter");
		ASBCharacter* NewCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter = NewCharacter;

		USBStateComponent* NewState = NewObject<USBStateComponent>(NewCharacter, TEXT("PersistentState"));
		NewState->RegisterComponent();
		USBInventoryComponent* NewInventory = NewObject<USBInventoryComponent>(NewCharacter, TEXT("PersistentInventory"));
		NewInventory->RegisterComponent();

		ISBComponentInterface::Execute_OnInitialize(NewState);
		ISBComponentInterface::Execute_OnInitialize(NewInventory);

		ISBComponentInterface::Execute_OnReady(NewState);
		ISBComponentInterface::Execute_OnReady(NewInventory);

		// 6. Carrega
		bool bLoadSuccess = SaveSubsystem->LoadGame(TEXT("TestWeaponSaveSlot"), 0);
		TestTrue("Carregamento deve ser bem-sucedido", bLoadSuccess);

		// 7. Simula a passagem do tick do timer manager para processar a re-equipagem deferida
		if (TestWorld)
		{
			TestWorld->GetTimerManager().Tick(0.01f);
		}

		// 8. Verifica que o item foi recarregado e re-equipado
		TArray<USBItemInstance*> LoadedItems = NewInventory->GetAllItems();
		TestEqual("Deve haver exatamente 1 item no inventário", LoadedItems.Num(), 1);
		if (LoadedItems.Num() > 0)
		{
			TestTrue("Item carregado deve possuir tag de equipado", LoadedItems[0]->DynamicTags.HasTag(EquippedTag));
			TestEqual("EquipCount após load (deferido no próximo tick) deve ser 1", EventListener->EquipCount, 1);
		}
	});
}
