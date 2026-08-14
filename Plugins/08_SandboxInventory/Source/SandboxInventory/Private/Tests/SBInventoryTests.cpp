#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Character/SBCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Player.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Components/SBStateComponent.h"
#include "Components/SBInventoryComponent.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemFragment.h"
#include "Items/SBItemFragment_Equippable.h"
#include "Subsystems/SBEventSubsystem.h"
#include "GameplayTagsManager.h"
#include "UObject/UnrealType.h"
#include "SBInventoryTestTypes.h"

BEGIN_DEFINE_SPEC(FSBInventoryTestsSpec, "Sandbox.Inventory", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBInventoryComponent* InventoryComponent;
	UActorComponent* CombatComponentRaw;
	UClass* CombatCompClass;
	USBTestInventoryListener* EventListener;
	
	APlayerController* ActiveController;
	ULocalPlayer* ActiveLocalPlayer;

	APlayerController* ActiveController2;
	ULocalPlayer* ActiveLocalPlayer2;
	ASBCharacter* TestCharacter2;
	USBInventoryComponent* InventoryComponent2;

	UGameInstance* GameInstance;
END_DEFINE_SPEC(FSBInventoryTestsSpec)

void FSBInventoryTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		GameInstance = NewObject<UGameInstance>(GEngine);
		GameInstance->InitializeStandalone();

		FWorldContext* WorldContext = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.OwningGameInstance == GameInstance)
			{
				WorldContext = const_cast<FWorldContext*>(&Context);
				break;
			}
		}
		if (!WorldContext)
		{
			FWorldContext& NewContext = GEngine->CreateNewWorldContext(EWorldType::Game);
			NewContext.OwningGameInstance = GameInstance;
			WorldContext = &NewContext;
		}
		if (WorldContext)
		{
			WorldContext->SetCurrentWorld(TestWorld);
		}

		TestWorld->SetGameInstance(GameInstance);

		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (TestCharacter && TestCharacter->GetCharacterMovement())
		{
			TestCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}

		// Instancia componentes no TestCharacter
		InventoryComponent = NewObject<USBInventoryComponent>(TestCharacter);
		InventoryComponent->RegisterComponent();

		// Instancia o CombatComponent via reflexão para evitar acoplamento de compilação
		CombatCompClass = FindObject<UClass>(nullptr, TEXT("/Script/SandboxCombat.SBCombatComponent"));
		if (CombatCompClass)
		{
			CombatComponentRaw = NewObject<UActorComponent>(TestCharacter, CombatCompClass);
			CombatComponentRaw->RegisterComponent();
			
			UFunction* InitFunc = CombatCompClass->FindFunctionByName(TEXT("OnInitialize"));
			if (InitFunc)
			{
				CombatComponentRaw->ProcessEvent(InitFunc, nullptr);
			}
		}

		// Registra a tag do teste se ela ainda não existir
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		if (!TagsManager.RequestGameplayTag(TEXT("Weapon.Tag.Pistol"), false).IsValid())
		{
			TagsManager.AddNativeGameplayTag(TEXT("Weapon.Tag.Pistol"), TEXT("Test pistol tag"));
		}

		// Registra escutador de eventos
		EventListener = NewObject<USBTestInventoryListener>();
		if (USBEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<USBEventSubsystem>())
		{
			EventSubsystem->SubscribeToEventNative(
				FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.SlotUpdated")),
				ESBEventPriority::Medium,
				FSBNativeEventDelegate::CreateUObject(EventListener, &USBTestInventoryListener::OnSlotUpdated)
			);
			EventSubsystem->SubscribeToEventNative(
				FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.ItemEquipped")),
				ESBEventPriority::Medium,
				FSBNativeEventDelegate::CreateUObject(EventListener, &USBTestInventoryListener::OnEquipped)
			);
			EventSubsystem->SubscribeToEventNative(
				FGameplayTag::RequestGameplayTag(TEXT("Event.Inventory.ItemUnequipped")),
				ESBEventPriority::Medium,
				FSBNativeEventDelegate::CreateUObject(EventListener, &USBTestInventoryListener::OnUnequipped)
			);
		}

		// Configuração de possessão e player controller para testes de autoridade local
		ActiveController = TestWorld->SpawnActor<APlayerController>();
		ActiveLocalPlayer = NewObject<ULocalPlayer>(GEngine);
		ActiveLocalPlayer->SetControllerId(0);
		ActiveController->Player = ActiveLocalPlayer;
		ActiveController->InitPlayerState();
		ActiveController->Possess(TestCharacter);

		// Setup do Player 2
		TestCharacter2 = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		InventoryComponent2 = NewObject<USBInventoryComponent>(TestCharacter2);
		InventoryComponent2->RegisterComponent();

		ActiveController2 = TestWorld->SpawnActor<APlayerController>();
		ActiveLocalPlayer2 = NewObject<ULocalPlayer>(GEngine);
		ActiveLocalPlayer2->SetControllerId(1);
		ActiveController2->Player = ActiveLocalPlayer2;
		ActiveController2->InitPlayerState();
		ActiveController2->Possess(TestCharacter2);
	});

	AfterEach([this]()
	{
		if (TestCharacter)
		{
			TestCharacter->Destroy();
			TestCharacter = nullptr;
		}
		if (TestCharacter2)
		{
			TestCharacter2->Destroy();
			TestCharacter2 = nullptr;
		}
		if (ActiveController)
		{
			ActiveController->Destroy();
			ActiveController = nullptr;
		}
		if (ActiveController2)
		{
			ActiveController2->Destroy();
			ActiveController2 = nullptr;
		}

		if (ActiveLocalPlayer)
		{
			ActiveLocalPlayer->MarkAsGarbage();
			ActiveLocalPlayer = nullptr;
		}
		if (ActiveLocalPlayer2)
		{
			ActiveLocalPlayer2->MarkAsGarbage();
			ActiveLocalPlayer2 = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}

		GEngine->DestroyWorldContext(TestWorld);
	});

	It("Cenário 1: Adição e Stacking de Itens", [this]()
	{
		USBItemDefinition* StackableItem = NewObject<USBItemDefinition>();
		StackableItem->DisplayName = FText::FromString(TEXT("Stackable Potion"));
		StackableItem->MaxStackCount = 5;

		// Adiciona 3 itens
		USBItemInstance* Inst1 = InventoryComponent->ServerAddItem(StackableItem, 3);
		TestTrue("Item instance criada", Inst1 != nullptr);
		TestEqual("Quantidade inicial", Inst1->StackCount, 3);
		TestEqual("Total de itens únicos no inventário", InventoryComponent->GetAllItems().Num(), 1);

		// Adiciona mais 4 itens (deve encher o primeiro slot até 5 e criar um segundo slot com 2)
		USBItemInstance* Inst2 = InventoryComponent->ServerAddItem(StackableItem, 4);
		TestTrue("Item instance da segunda pilha criada", Inst2 != nullptr);
		TestEqual("Pilha 1 cheia", Inst1->StackCount, 5);
		TestEqual("Pilha 2 com o resto", Inst2->StackCount, 2);
		TestEqual("Total de slots ocupados", InventoryComponent->GetAllItems().Num(), 2);
	});

	It("Cenário 2: Replicação de Slots e Subobjetos (Ordem de Chegada + Timeout)", [this]()
	{
		// Registra entrada manual vazia simulando replicação inacabada do subobjeto
		InventoryComponent->OnEntryReplicated(0);

		// Avança tempo e ticka 5 vezes de 0.5s para simular 2.5s e testar o timeout de 2.0s
		for (int32 i = 0; i < 5; ++i)
		{
			TestWorld->Tick(LEVELTICK_All, 0.5f);
			InventoryComponent->TickComponent(0.5f, LEVELTICK_All, nullptr);
		}

		// Fila deve estar limpa e nenhum evento de slot disparado para UI
		TestEqual("Slot não disparado", EventListener->SlotUpdateCount, 0);
	});

	It("Cenário 3: Integração de Equipar / Fragments", [this]()
	{
		UClass* HitscanClass = FindObject<UClass>(nullptr, TEXT("/Script/SandboxCombat.SBWeaponBehaviorHitscan"));
		UClass* WeaponDefClass = FindObject<UClass>(nullptr, TEXT("/Script/SandboxCombat.SBWeaponBehaviorDefinition"));

		TestTrue("Hitscan behavior class existe", HitscanClass != nullptr);
		TestTrue("WeaponBehaviorDefinition class existe", WeaponDefClass != nullptr);

		USBItemDefinition* WeaponItem = NewObject<USBItemDefinition>();
		WeaponItem->DisplayName = FText::FromString(TEXT("Rifle"));
		WeaponItem->MaxStackCount = 1;

		USBItemFragment_Equippable* EquipFrag = NewObject<USBItemFragment_Equippable>(WeaponItem);
		EquipFrag->WeaponBehaviorClass = HitscanClass;
		
		UPrimaryDataAsset* WeaponDef = Cast<UPrimaryDataAsset>(NewObject<UObject>(GetTransientPackage(), WeaponDefClass));
		FStructProperty* TagProp = CastField<FStructProperty>(WeaponDefClass->FindPropertyByName(TEXT("BehaviorTag")));
		FGameplayTag WeaponTag = FGameplayTag::RequestGameplayTag(TEXT("Weapon.Tag.Pistol"));
		TagProp->SetValue_InContainer(WeaponDef, &WeaponTag);
		
		EquipFrag->WeaponDefinitionAsset = WeaponDef;
		WeaponItem->Fragments.Add(EquipFrag);

		USBItemInstance* Inst = InventoryComponent->ServerAddItem(WeaponItem, 1);
		TestTrue("Item rifle criado", Inst != nullptr);

		// Simula equipar
		InventoryComponent->ServerEquipItem(Inst);

		// Verifica se o CombatComponent escutou o evento via Message Router e instanciou a arma
		TestEqual(TEXT("Equip event disparado"), EventListener->EquipCount, 1);
		TestEqual(TEXT("Payload recebido"), EventListener->LastEquippedInstance.Get(), Inst);

		FArrayProperty* AvailableWeaponsProp = CastField<FArrayProperty>(CombatCompClass->FindPropertyByName(TEXT("AvailableBehaviors")));
		FScriptArrayHelper ArrayHelper(AvailableWeaponsProp, AvailableWeaponsProp->ContainerPtrToValuePtr<void>(CombatComponentRaw));
		TestEqual(TEXT("Arma disponível instanciada no CombatComponent"), ArrayHelper.Num(), 1);

		UObject* SpawnedBehavior = ArrayHelper.IsValidIndex(0) ? (ArrayHelper.GetRawPtr(0) ? *reinterpret_cast<UObject**>(ArrayHelper.GetRawPtr(0)) : nullptr) : nullptr;
		TestTrue("Behavior de arma instanciado", SpawnedBehavior != nullptr);

		FObjectProperty* DefAssetProp = CastField<FObjectProperty>(SpawnedBehavior->GetClass()->FindPropertyByName(TEXT("WeaponDefinition")));
		UObject* SpawnedDef = DefAssetProp->GetPropertyValue_InContainer(SpawnedBehavior);
		FStructProperty* SpawnedTagProp = CastField<FStructProperty>(SpawnedDef->GetClass()->FindPropertyByName(TEXT("BehaviorTag")));
		FGameplayTag SpawnedTag = *SpawnedTagProp->ContainerPtrToValuePtr<FGameplayTag>(SpawnedDef);
		TestEqual(TEXT("Tag do behavior da arma bate"), SpawnedTag, WeaponTag);
	});

	It("Cenário 4: Prevenção de Condição de Corrida (Loot Dispute)", [this]()
	{
		USBItemDefinition* RareLoot = NewObject<USBItemDefinition>();
		RareLoot->DisplayName = FText::FromString(TEXT("Rare Ring"));
		RareLoot->MaxStackCount = 1;

		// Spawn do Loot Pickup Actor compartilhado no mundo
		ASBTestLootPickupActor* Pickup = TestWorld->SpawnActor<ASBTestLootPickupActor>();
		Pickup->LootItemDef = RareLoot;

		// Jogador 1 inicia interação e adquire lock
		TestFalse("Loot inicial não bloqueado", Pickup->IsInteractionLocked_Implementation(TestCharacter));
		Pickup->LockInteraction_Implementation(TestCharacter);

		// Jogador 2 tenta interagir ao mesmo tempo
		TestTrue("Loot agora bloqueado para Jogador 2", Pickup->IsInteractionLocked_Implementation(TestCharacter2));

		// Jogador 1 finaliza interação (Completa e destrói o pickup)
		Pickup->Interact_Implementation(TestCharacter);

		// Verifica inventário do Jogador 1 e Jogador 2
		TestEqual(TEXT("Jogador 1 recebeu o anel"), InventoryComponent->GetAllItems().Num(), 1);
		TestEqual(TEXT("Jogador 2 não tem itens no inventário"), InventoryComponent2->GetAllItems().Num(), 0);
	});

	It("Cenário 5: Desequipamento e Ejeção Simétrica", [this]()
	{
		UClass* HitscanClass = FindObject<UClass>(nullptr, TEXT("/Script/SandboxCombat.SBWeaponBehaviorHitscan"));
		UClass* WeaponDefClass = FindObject<UClass>(nullptr, TEXT("/Script/SandboxCombat.SBWeaponBehaviorDefinition"));

		USBItemDefinition* WeaponItem = NewObject<USBItemDefinition>();
		USBItemFragment_Equippable* EquipFrag = NewObject<USBItemFragment_Equippable>(WeaponItem);
		EquipFrag->WeaponBehaviorClass = HitscanClass;
		
		UPrimaryDataAsset* WeaponDef = Cast<UPrimaryDataAsset>(NewObject<UObject>(GetTransientPackage(), WeaponDefClass));
		FStructProperty* TagProp = CastField<FStructProperty>(WeaponDefClass->FindPropertyByName(TEXT("BehaviorTag")));
		FGameplayTag WeaponTag = FGameplayTag::RequestGameplayTag(TEXT("Weapon.Tag.Pistol"));
		TagProp->SetValue_InContainer(WeaponDef, &WeaponTag);
		EquipFrag->WeaponDefinitionAsset = WeaponDef;
		WeaponItem->Fragments.Add(EquipFrag);

		USBItemInstance* Inst = InventoryComponent->ServerAddItem(WeaponItem, 1);
		
		// Equipar
		InventoryComponent->ServerEquipItem(Inst);

		FArrayProperty* AvailableWeaponsProp = CastField<FArrayProperty>(CombatCompClass->FindPropertyByName(TEXT("AvailableBehaviors")));
		FScriptArrayHelper ArrayHelper(AvailableWeaponsProp, AvailableWeaponsProp->ContainerPtrToValuePtr<void>(CombatComponentRaw));
		TestEqual(TEXT("Arma instanciada"), ArrayHelper.Num(), 1);

		// Desequipar
		InventoryComponent->ServerUnequipItem(Inst);
		TestEqual(TEXT("Unequip event disparado"), EventListener->UnequipCount, 1);
		
		FScriptArrayHelper ArrayHelperPost(AvailableWeaponsProp, AvailableWeaponsProp->ContainerPtrToValuePtr<void>(CombatComponentRaw));
		TestEqual(TEXT("Arma desinstalada do CombatComponent"), ArrayHelperPost.Num(), 0);
	});
}
