#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/SBQuestComponent.h"
#include "Components/SBMerchantComponent.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBExperienceComponent.h"
#include "DataAssets/SBQuestDataAsset.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemInstance.h"
#include "Subsystems/SBEventSubsystem.h"
#include "Subsystems/SBEventPayloads.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBQuestMerchantTestsSpec, "Sandbox.QuestsAndMerchant", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	UGameInstance* GameInstance;
	ACharacter* PlayerCharacter;
	AActor* MerchantActor;
	USBQuestComponent* QuestComp;
	USBExperienceComponent* XPComp;
	USBInventoryComponent* InvComp;
	USBAttributeComponent* AttrComp;
	USBMerchantComponent* MerchantComp;
	USBQuestDataAsset* TestQuest;
	USBItemDefinition* TestItem;
END_DEFINE_SPEC(FSBQuestMerchantTestsSpec)

void FSBQuestMerchantTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.SetCurrentWorld(TestWorld);

		GameInstance = NewObject<UGameInstance>(GEngine);
		WorldContext.OwningGameInstance = GameInstance;
		GameInstance->InitializeStandalone();
		TestWorld->SetGameInstance(GameInstance);

		FSBGameplayTags::InitializeNativeTags();

		// 1. Cria Ator do Jogador e anexa componentes lógicos
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("TestPlayerPawn");
		PlayerCharacter = TestWorld->SpawnActor<ACharacter>(ACharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		QuestComp = NewObject<USBQuestComponent>(PlayerCharacter, TEXT("QuestComponent"));
		QuestComp->RegisterComponent();

		XPComp = NewObject<USBExperienceComponent>(PlayerCharacter, TEXT("ExperienceComponent"));
		XPComp->RegisterComponent();

		InvComp = NewObject<USBInventoryComponent>(PlayerCharacter, TEXT("InventoryComponent"));
		InvComp->RegisterComponent();

		AttrComp = NewObject<USBAttributeComponent>(PlayerCharacter, TEXT("AttributeComponent"));
		AttrComp->RegisterComponent();

		// Inicializa o atributo de moedas/Coins
		FSBAttribute CoinsAttr;
		CoinsAttr.BaseValue = 100.0f;
		CoinsAttr.MinValue = 0.0f;
		CoinsAttr.MaxValue = 9999.0f;
		CoinsAttr.bIsPrivate = true;
		AttrComp->RegisterAttribute(FSBGameplayTags::Get().Attribute_Coins, CoinsAttr);

		// Dispara a inicialização e o BeginPlay dos componentes do player
		PlayerCharacter->DispatchBeginPlay();

		// 2. Cria Ator Comerciante e anexa componente de comércio
		FActorSpawnParameters MerchSpawnParams;
		MerchSpawnParams.Name = TEXT("TestMerchantActor");
		MerchantActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, MerchSpawnParams);
		// AActor puro nao tem RootComponent no momento do spawn, entao a posicao passada a
		// SpawnActor nao e aplicada. Posicionar explicitamente apos o root existir.
		USceneComponent* MerchantActorRoot = NewObject<USceneComponent>(MerchantActor, TEXT("MerchantActorRoot"));
		MerchantActor->SetRootComponent(MerchantActorRoot);
		MerchantActorRoot->RegisterComponent();
		MerchantActor->SetActorLocation(FVector(100.0f, 0.0f, 0.0f));

		MerchantComp = NewObject<USBMerchantComponent>(MerchantActor, TEXT("MerchantComponent"));
		MerchantComp->RegisterComponent();

		MerchantActor->DispatchBeginPlay();

		// 3. Cria Item Definition de teste
		TestItem = NewObject<USBItemDefinition>(TestWorld, TEXT("TestMerchantItem"));
		const_cast<FText&>(TestItem->DisplayName) = FText::FromString(TEXT("Madeira Mágica"));
		const_cast<int32&>(TestItem->MaxStackCount) = 99;
		const_cast<FGameplayTagContainer&>(TestItem->ItemTags).AddTag(FSBGameplayTags::Get().Item_Type_Material);

		// Configura estoque do comerciante (Preço = 30)
		MerchantComp->AvailableItems.Add(TestItem, 30);

		// 4. Cria Data Asset de Quest de teste
		TestQuest = NewObject<USBQuestDataAsset>(TestWorld, TEXT("TestQuestAsset"));
		TestQuest->QuestName = FText::FromString(TEXT("Caminhada Mágica"));

		// Objetivo: Dar 3 passos
		FSBQuestObjective StepObjective;
		StepObjective.ObjectiveTag = FSBGameplayTags::Get().Quest_Objective_Footstep;
		StepObjective.RequiredCount = 3;
		StepObjective.Description = FText::FromString(TEXT("Dê 3 passos físicos"));
		TestQuest->Objectives.Add(StepObjective);

		// Recompensas: 50 XP e 2 Wood Items
		FSBQuestReward XPReward;
		XPReward.Experience = 50.0f;
		TestQuest->Rewards.Add(XPReward);

		FSBQuestReward ItemReward;
		ItemReward.ItemDef = TestItem;
		ItemReward.Quantity = 2;
		TestQuest->Rewards.Add(ItemReward);
	});

	AfterEach([this]()
	{
		if (PlayerCharacter)
		{
			PlayerCharacter->Destroy();
			PlayerCharacter = nullptr;
		}
		if (MerchantActor)
		{
			MerchantActor->Destroy();
			MerchantActor = nullptr;
		}
		if (GameInstance)
		{
			GameInstance->Shutdown();
			GameInstance = nullptr;
		}
		GEngine->DestroyWorldContext(TestWorld);
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	Describe("Quest System Logic Verification", [this]()
	{
		It("Should progress quest objectives via Event Bus and grant rewards on claim", [this]()
		{
			// 1. Aceita missão
			QuestComp->AcceptQuest(TestQuest);
			TestTrue("Quest deve estar ativa", QuestComp->IsQuestActive(TestQuest));
			TestFalse("Quest não deve estar concluída inicialmente", QuestComp->IsQuestCompleted(TestQuest));

			// 2. Simula o barramento disparando passos físicos
			UGameInstance* GI = TestWorld->GetGameInstance();
			USBEventSubsystem* EventSubsystem = GI ? GI->GetSubsystem<USBEventSubsystem>() : nullptr;
			TestNotNull("EventSubsystem deve existir", EventSubsystem);

			if (EventSubsystem)
			{
				USBFootstepEventPayload* Payload = NewObject<USBFootstepEventPayload>(EventSubsystem);
				Payload->TargetPawn = Cast<APawn>(PlayerCharacter);
				Payload->Location = FVector::ZeroVector;
				Payload->FootSocketName = TEXT("foot_l");

				// Dispara 3 eventos de passos
				EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Character_Footstep, Payload);
				EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Character_Footstep, Payload);
				EventSubsystem->PublishEvent(FSBGameplayTags::Get().Event_Character_Footstep, Payload);
			}

			// 3. Verifica conclusão da missão
			TestTrue("Quest deve ser marcada como concluída após os 3 passos", QuestComp->IsQuestCompleted(TestQuest));

			// 4. Reivindica as recompensas (XP e Itens)
			int32 OldXP = XPComp->GetCurrentXP();
			int32 OldItemQty = InvComp->GetTotalItemQuantity(TestItem);

			bool bClaimed = QuestComp->ClaimQuestRewards(TestQuest);
			TestTrue("Reivindicação de recompensa deve retornar true", bClaimed);

			// Confirma que XP foi somado (50 XP)
			TestEqual("XP deve ter aumentado em 50", XPComp->GetCurrentXP(), OldXP + 50);

			// Confirma que os itens de recompensa foram adicionados ao inventário de forma desacoplada
			TestEqual("Inventário do jogador deve conter 2 itens adicionados do prêmio", InvComp->GetTotalItemQuantity(TestItem), OldItemQty + 2);
		});
	});

	Describe("Merchant Component Transaction Verification", [this]()
	{
		It("Should execute valid purchases and reject transactions if coins are insufficient or distance exceeds limits", [this]()
		{
			// Preço = 30 Moedas. Saldo inicial = 100 Moedas.
			FGameplayTag CoinsTag = FSBGameplayTags::Get().Attribute_Coins;

			// 1. Compra válida (1 item)
			MerchantComp->ServerBuyItem(Cast<APawn>(PlayerCharacter), TestItem, 1);

			// Payout esperado: Moedas = 70, Inventário = 1
			TestEqual("Saldo de moedas deve ser 70", AttrComp->GetAttributeValue(CoinsTag), 70.0f);
			TestEqual("Total de itens comprados no inventário deve ser 1", InvComp->GetTotalItemQuantity(TestItem), 1);

			// 2. Compra inválida (Tenta comprar 3 unidades por 90 moedas, mas saldo é 70)
			MerchantComp->ServerBuyItem(Cast<APawn>(PlayerCharacter), TestItem, 3);

			// Deve rejeitar a transação: moedas continuam 70, inventário continua 1
			TestEqual("Saldo deve permanecer 70 após compra negada", AttrComp->GetAttributeValue(CoinsTag), 70.0f);
			TestEqual("Total de itens não deve mudar", InvComp->GetTotalItemQuantity(TestItem), 1);

			// 3. Venda legítima de item
			TArray<USBItemInstance*> InventoryItems = InvComp->GetAllItems();
			TestTrue("Deve conter pelo menos um item no inventário", InventoryItems.Num() > 0);

			if (InventoryItems.Num() > 0)
			{
				// Vende o item comprado. Payout esperado: 30 moedas * 0.5f (multiplier) = 15 moedas.
				// Saldo deve ir para 70 + 15 = 85. Itens devem ir para 0.
				MerchantComp->ServerSellItem(Cast<APawn>(PlayerCharacter), InventoryItems[0], 1);

				TestEqual("Saldo de moedas pós venda deve ser 85", AttrComp->GetAttributeValue(CoinsTag), 85.0f);
				TestEqual("Total de itens no inventário pós venda deve ser 0", InvComp->GetTotalItemQuantity(TestItem), 0);
			}

			// 4. Anti-Cheat: Afasta o jogador do comerciante (Distância de 5000 unidades)
			PlayerCharacter->SetActorLocation(FVector(5000.0f, 0.0f, 0.0f));

			// Tenta comprar novamente longe do alcance
			MerchantComp->ServerBuyItem(Cast<APawn>(PlayerCharacter), TestItem, 1);

			// O saldo deve permanecer inalterado (85 moedas)
			TestEqual("Saldo deve permanecer 85 devido a cheat de distância", AttrComp->GetAttributeValue(CoinsTag), 85.0f);
		});
	});
}
