// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Character/SBCharacter.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBStateComponent.h"
#include "Actors/SBCraftingStation.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"

BEGIN_DEFINE_SPEC(FSBCraftingStationTestsSpec, "Sandbox.CraftingStation", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ASBCharacter* TestCharacter;
	USBInventoryComponent* InventoryComponent;
	USBStateComponent* StateComponent;
	ASBCraftingStation* CraftingStation;
	FGameplayTag StationForgeTag;
END_DEFINE_SPEC(FSBCraftingStationTestsSpec)

void FSBCraftingStationTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestCharacter = TestWorld->SpawnActor<ASBCharacter>(ASBCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TestCharacter->SetRole(ROLE_Authority);

		InventoryComponent = NewObject<USBInventoryComponent>(TestCharacter);
		InventoryComponent->RegisterComponent();
		
		StateComponent = NewObject<USBStateComponent>(TestCharacter);
		StateComponent->RegisterComponent();

		// Registra Tag de Teste
		UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
		StationForgeTag = TagsManager.AddNativeGameplayTag(TEXT("Crafting.Station.Forge"));

		// Spawn da Estação
		CraftingStation = TestWorld->SpawnActor<ASBCraftingStation>(ASBCraftingStation::StaticClass(), FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParams);
		CraftingStation->StationTag = StationForgeTag;
		CraftingStation->MaxInteractionDistance = 250.0f;
	});

	AfterEach([this]()
	{
		if (CraftingStation)
		{
			CraftingStation->Destroy();
			CraftingStation = nullptr;
		}
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

	It("ASBCraftingStation deve permitir configurar a StationTag", [this]()
	{
		TestEqual("StationTag deve ser a tag de Forge", CraftingStation->StationTag, StationForgeTag);
	});

	It("Interact com a bancada deve conceder a StationTag ao interator", [this]()
	{
		TestFalse("Jogador nao deve comecar com a tag", StateComponent->HasTag(StationForgeTag));

		// Interage
		CraftingStation->Interact_Implementation(TestCharacter);

		TestTrue("Jogador deve ganhar a tag de forge apos interacao", StateComponent->HasTag(StationForgeTag));
	});

	It("StopInteracting deve retirar a StationTag do interator", [this]()
	{
		CraftingStation->Interact_Implementation(TestCharacter);
		TestTrue("Jogador possui a tag", StateComponent->HasTag(StationForgeTag));

		// Para interacao
		CraftingStation->StopInteracting(TestCharacter);

		TestFalse("Jogador deve perder a tag apos StopInteracting", StateComponent->HasTag(StationForgeTag));
	});

	It("Tick da bancada deve retirar a tag se o interator se mover para fora do raio MaxInteractionDistance", [this]()
	{
		CraftingStation->Interact_Implementation(TestCharacter);
		TestTrue("Jogador comeca com a tag de forge", StateComponent->HasTag(StationForgeTag));

		// Move o jogador para fora do raio (300 unidades de distancia)
		TestCharacter->SetActorLocation(FVector(500.f, 0.f, 0.f));

		// Roda o Tick da bancada
		CraftingStation->Tick(0.1f);

		TestFalse("Jogador deve perder a tag ao se afastar alem de MaxInteractionDistance", StateComponent->HasTag(StationForgeTag));
	});
}
