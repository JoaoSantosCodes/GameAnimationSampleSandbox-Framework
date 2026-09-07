// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/SBComboComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBComboTestsSpec, "Sandbox.Combat.ComboSystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBStateComponent* StateComp;
	USBComboComponent* ComboComp;
	FSBComboTree SampleTree;
END_DEFINE_SPEC(FSBComboTestsSpec)

void FSBComboTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		TestActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		// Um AActor puro nao tem RootComponent: sem ele, SetActorLocation, SetActorTransform
		// e TeleportTo falham em silencio e o ator fica preso na origem.
		USceneComponent* TestActorRoot = NewObject<USceneComponent>(TestActor, TEXT("TestActorRoot"));
		TestActor->SetRootComponent(TestActorRoot);
		TestActorRoot->RegisterComponent();

		StateComp = NewObject<USBStateComponent>(TestActor, TEXT("TestState"));
		StateComp->RegisterComponent();
		TestActor->AddOwnedComponent(StateComp);

		ComboComp = NewObject<USBComboComponent>(TestActor, TEXT("TestCombo"));
		ComboComp->RegisterComponent();
		TestActor->AddOwnedComponent(ComboComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(ComboComp);

		// Configura árvore de combo de teste:
		// Node 0 (Light 1) -> Node 1 (Light 2) OU Node 10 (Heavy 1)
		// Node 1 (Light 2) -> Node 2 (Light 3 / Finisher)
		// Node 10 (Heavy 1) -> Node 11 (Heavy Finisher)
		SampleTree.ComboTreeTag = FSBGameplayTags::Get().Combat_Combo_Light;
		SampleTree.MaxWindowDuration = 1.5f;

		FSBComboNode Node0;
		Node0.NodeId = 0;
		Node0.ExpectedInput = ESBComboInputType::LightAttack;
		Node0.DamageMultiplier = 1.0f;
		Node0.BranchTargetNodeIds = { 1, 10 };
		Node0.bIsFinisher = false;

		FSBComboNode Node1;
		Node1.NodeId = 1;
		Node1.ExpectedInput = ESBComboInputType::LightAttack;
		Node1.DamageMultiplier = 1.25f;
		Node1.BranchTargetNodeIds = { 2 };
		Node1.bIsFinisher = false;

		FSBComboNode Node2;
		Node2.NodeId = 2;
		Node2.ExpectedInput = ESBComboInputType::LightAttack;
		Node2.DamageMultiplier = 2.0f;
		Node2.bIsFinisher = true;

		FSBComboNode Node10;
		Node10.NodeId = 10;
		Node10.ExpectedInput = ESBComboInputType::HeavyAttack;
		Node10.DamageMultiplier = 1.5f;
		Node10.BranchTargetNodeIds = { 11 };
		Node10.bIsFinisher = false;

		FSBComboNode Node11;
		Node11.NodeId = 11;
		Node11.ExpectedInput = ESBComboInputType::HeavyAttack;
		Node11.DamageMultiplier = 3.0f;
		Node11.bIsFinisher = true;

		SampleTree.Nodes = { Node0, Node1, Node2, Node10, Node11 };
		ComboComp->RegisterComboTree(SampleTree);
	});

	AfterEach([this]()
	{
		if (TestActor)
		{
			TestActor->Destroy();
			TestActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should progress through linear light combo sequence", [this]()
	{
		// 1. Golpe 1 (Light)
		bool bHit1 = ComboComp->ProcessComboInput(ESBComboInputType::LightAttack);
		TestTrue("Initial light attack should start combo", bHit1);
		TestEqual("Current node should be 0", ComboComp->GetCurrentNodeId(), 0);
		TestEqual("Combo count should be 1", ComboComp->GetComboCounter(), 1);
		TestEqual("Damage multiplier should be 1.0", ComboComp->GetCurrentDamageMultiplier(), 1.0f);

		// 2. Abre janela e desfere Golpe 2 (Light)
		ComboComp->OpenComboWindow();
		bool bHit2 = ComboComp->ProcessComboInput(ESBComboInputType::LightAttack);
		TestTrue("Chained light attack should succeed", bHit2);
		TestEqual("Current node should be 1", ComboComp->GetCurrentNodeId(), 1);
		TestEqual("Combo count should be 2", ComboComp->GetComboCounter(), 2);
		TestEqual("Damage multiplier should be 1.25", ComboComp->GetCurrentDamageMultiplier(), 1.25f);

		// 3. Abre janela e desfere Golpe 3 (Finisher)
		ComboComp->OpenComboWindow();
		bool bHit3 = ComboComp->ProcessComboInput(ESBComboInputType::LightAttack);
		TestTrue("Finisher light attack should succeed", bHit3);
		// Finisher conclui o combo e reseta
		TestEqual("Combo should be reset after finisher", ComboComp->GetCurrentNodeId(), INDEX_NONE);
		TestEqual("Combo count should be 0", ComboComp->GetComboCounter(), 0);
	});

	It("Should branch combo into heavy attack path", [this]()
	{
		// 1. Light 1
		ComboComp->ProcessComboInput(ESBComboInputType::LightAttack);
		TestEqual("Node 0 started", ComboComp->GetCurrentNodeId(), 0);

		// 2. Abre janela e ramifica para Heavy Attack
		ComboComp->OpenComboWindow();
		bool bBranch = ComboComp->ProcessComboInput(ESBComboInputType::HeavyAttack);
		TestTrue("Branch to heavy attack should succeed", bBranch);
		TestEqual("Current node should be 10 (Heavy 1)", ComboComp->GetCurrentNodeId(), 10);
		TestEqual("Damage multiplier should be 1.5", ComboComp->GetCurrentDamageMultiplier(), 1.5f);
	});

	It("Should buffer input and execute automatically when combo window opens", [this]()
	{
		// 1. Inicia Light 1 (janela fechada)
		ComboComp->ProcessComboInput(ESBComboInputType::LightAttack);
		TestFalse("Window should initially be closed", ComboComp->IsWithinComboWindow());

		// 2. Aperta botão de ataque enquanto a janela ainda está fechada (bufferiza)
		ComboComp->ProcessComboInput(ESBComboInputType::LightAttack);
		TestEqual("Should remain at Node 0 until window opens", ComboComp->GetCurrentNodeId(), 0);

		// 3. Notificação de animação abre a janela -> deve avançar instantaneamente para Node 1
		ComboComp->OpenComboWindow();
		TestEqual("Buffered input should advance combo to Node 1 on window open", ComboComp->GetCurrentNodeId(), 1);
		TestEqual("Combo count should be 2", ComboComp->GetComboCounter(), 2);
	});

	It("Should reset combo sequence when window timer expires", [this]()
	{
		// 1. Inicia Light 1
		ComboComp->ProcessComboInput(ESBComboInputType::LightAttack);
		TestEqual("Node 0 active", ComboComp->GetCurrentNodeId(), 0);

		// 2. Simula tempo sem inputs excedendo MaxWindowDuration (1.5s)
		ComboComp->TickComponent(2.0f, ELevelTick::LEVELTICK_All, nullptr);

		TestEqual("Combo should be reset after timer expires", ComboComp->GetCurrentNodeId(), INDEX_NONE);
		TestEqual("Combo counter should be 0", ComboComp->GetComboCounter(), 0);
	});
}
