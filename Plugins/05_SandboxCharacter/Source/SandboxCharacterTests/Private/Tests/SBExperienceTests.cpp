// Copyright 2026 João Santos. All Rights Reserved.
// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Components/SBExperienceComponent.h"

BEGIN_DEFINE_SPEC(FSBExperienceTestsSpec, "Sandbox.Experience", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* TestActor;
	USBExperienceComponent* ExperienceComponent;
END_DEFINE_SPEC(FSBExperienceTestsSpec)

void FSBExperienceTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		
		FActorSpawnParameters SpawnParams;
		TestActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		// Um AActor puro nao tem RootComponent: sem ele, SetActorLocation, SetActorTransform
		// e TeleportTo falham em silencio e o ator fica preso na origem.
		USceneComponent* TestActorRoot = NewObject<USceneComponent>(TestActor, TEXT("TestActorRoot"));
		TestActor->SetRootComponent(TestActorRoot);
		TestActorRoot->RegisterComponent();
		TestActor->SetRole(ROLE_Authority);

		ExperienceComponent = NewObject<USBExperienceComponent>(TestActor);
		ExperienceComponent->RegisterComponent();
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
			TestWorld->DestroyWorld(true);
			TestWorld = nullptr;
		}
	});

	It("USBExperienceComponent deve inicializar corretamente com nivel 1 e XP 0", [this]()
	{
		TestEqual("Nível inicial deve ser 1", ExperienceComponent->GetCurrentLevel(), 1);
		TestEqual("XP inicial deve ser 0", ExperienceComponent->GetCurrentXP(), 0);
		TestTrue("RequiredXP deve ser maior que 0", ExperienceComponent->GetRequiredXP() > 0);
	});

	It("AddExperience deve somar XP e propagar delegate OnExperienceChanged", [this]()
	{
		bool bChangedCalled = false;
		int32 BroadcastXP = 0;
		int32 BroadcastDelta = 0;

		ExperienceComponent->OnExperienceChanged.AddLambda([&](int32 NewXP, int32 DeltaXP, int32 ReqXP)
		{
			bChangedCalled = true;
			BroadcastXP = NewXP;
			BroadcastDelta = DeltaXP;
		});

		// Adiciona XP abaixo do limite (que é BaseRequiredXP * 1^1.5 = 100)
		ExperienceComponent->AddExperience(40);

		TestTrue("OnExperienceChanged deve ser acionado", bChangedCalled);
		TestEqual("XP atual deve ser 40", ExperienceComponent->GetCurrentXP(), 40);
		TestEqual("BroadcastXP deve ser 40", BroadcastXP, 40);
		TestEqual("BroadcastDelta deve ser 40", BroadcastDelta, 40);
		TestEqual("Nível deve continuar 1", ExperienceComponent->GetCurrentLevel(), 1);
	});

	It("AddExperience deve realizar level up simples e recalcular RequiredXP com carry-over", [this]()
	{
		bool bLevelUpCalled = false;
		int32 NewLevelBroadcast = 0;

		ExperienceComponent->OnLevelUp.AddLambda([&](int32 NewLevel)
		{
			bLevelUpCalled = true;
			NewLevelBroadcast = NewLevel;
		});

		// Nível 1 exige 100 XP. Adicionamos 120 XP.
		// Deve ir para o Nível 2 com 20 XP restante.
		ExperienceComponent->AddExperience(120);

		TestTrue("OnLevelUp deve ser acionado", bLevelUpCalled);
		TestEqual("Nível final deve ser 2", ExperienceComponent->GetCurrentLevel(), 2);
		TestEqual("Broadcast do novo nível deve ser 2", NewLevelBroadcast, 2);
		TestEqual("XP atual deve ser 20 (carry-over)", ExperienceComponent->GetCurrentXP(), 20);

		// Nível 2 exige BaseRequiredXP * 2^1.5 = 100 * 2.8284 = 283 XP (RoundToInt)
		TestEqual("RequiredXP recalcula para o Nível 2", ExperienceComponent->GetRequiredXP(), 283);
	});

	It("AddExperience deve realizar multiplos level ups em cadeia com carry-over correto", [this]()
	{
		TArray<int32> LevelUpLevels;
		ExperienceComponent->OnLevelUp.AddLambda([&](int32 NewLevel)
		{
			LevelUpLevels.Add(NewLevel);
		});

		// Nível 1 exige 100 XP. (Novo RequiredXP = 283)
		// Nível 2 exige 283 XP. (Novo RequiredXP = 520)
		// Nível 3 exige 520 XP. (Novo RequiredXP = 800)
		// Total necessário para ir do Nível 1 ao Nível 4: 100 + 283 + 520 = 903 XP.
		// Vamos injetar 950 XP.
		// Deve atingir Nível 4, restando 950 - 903 = 47 XP de carry-over.
		ExperienceComponent->AddExperience(950);

		TestEqual("Nível final deve ser 4", ExperienceComponent->GetCurrentLevel(), 4);
		TestEqual("Devem ocorrer 3 level ups", LevelUpLevels.Num(), 3);
		if (LevelUpLevels.Num() == 3)
		{
			TestEqual("Primeiro level up deve ser para o nível 2", LevelUpLevels[0], 2);
			TestEqual("Segundo level up deve ser para o nível 3", LevelUpLevels[1], 3);
			TestEqual("Terceiro level up deve ser para o nível 4", LevelUpLevels[2], 4);
		}
		TestEqual("XP atual (carry-over) deve ser 47", ExperienceComponent->GetCurrentXP(), 47);
	});

	It("GetRequiredXPForLevel deve usar DataTable se configurado e retornar formula como fallback", [this]()
	{
		UDataTable* TestDataTable = NewObject<UDataTable>(ExperienceComponent);
		TestDataTable->RowStruct = FRequiredXPRow::StaticStruct();

		FRequiredXPRow Row1;
		Row1.RequiredXP = 50;
		TestDataTable->AddRow(FName("1"), Row1);

		FRequiredXPRow Row2;
		Row2.RequiredXP = 150;
		TestDataTable->AddRow(FName("2"), Row2);

		// Configura o DataTable no componente e atualiza
		ExperienceComponent->SetRequiredXPDataTable(TestDataTable);

		// Nível 1 deve usar o valor do DataTable (50)
		TestEqual("XP requerido para nível 1 deve ser 50", ExperienceComponent->GetRequiredXPForLevel(1), 50);

		// Nível 2 deve usar o valor do DataTable (150)
		TestEqual("XP requerido para nível 2 deve ser 150", ExperienceComponent->GetRequiredXPForLevel(2), 150);

		// Nível 3 não está no DataTable, deve fazer fallback para a fórmula:
		// BaseRequiredXP * 3^1.5 = 100 * 5.196 = 520 (RoundToInt)
		TestEqual("XP requerido para nível 3 (fallback) deve ser 520", ExperienceComponent->GetRequiredXPForLevel(3), 520);
	});
}
