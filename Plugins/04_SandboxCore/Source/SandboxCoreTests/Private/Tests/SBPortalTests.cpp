// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Subsystems/SBPortalSubsystem.h"
#include "Actors/SBPortalActor.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBPortalTestsSpec, "Sandbox.Portals", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	USBPortalSubsystem* PortalSubsystem;
	ASBPortalActor* PortalA;
	ASBPortalActor* PortalB;
	AActor* TestActor;
END_DEFINE_SPEC(FSBPortalTestsSpec)

void FSBPortalTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));
		PortalSubsystem = TestWorld->GetSubsystem<USBPortalSubsystem>();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 1. Cria Portal A na origem (0, 0, 0)
		PortalA = TestWorld->SpawnActor<ASBPortalActor>(ASBPortalActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		PortalA->PortalInfo.PortalTag = FSBGameplayTags::Get().Portal_Type_Gateway;
		PortalA->PortalInfo.bIsOpen = true;
		PortalA->PortalInfo.bIsLocked = false;
		PortalSubsystem->RegisterPortal(PortalA);

		// 2. Cria Portal B em (1000, 2000, 100)
		PortalB = TestWorld->SpawnActor<ASBPortalActor>(ASBPortalActor::StaticClass(), FVector(1000.0f, 2000.0f, 100.0f), FRotator::ZeroRotator, SpawnParams);
		PortalB->PortalInfo.PortalTag = FSBGameplayTags::Get().Portal_Type_Waystone;
		PortalB->PortalInfo.bIsOpen = true;
		PortalB->PortalInfo.bIsLocked = false;
		PortalSubsystem->RegisterPortal(PortalB);

		// 3. Cria Ator de teste
		TestActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		// Um AActor puro nao tem RootComponent: sem ele, SetActorLocation, SetActorTransform
		// e TeleportTo falham em silencio e o ator fica preso na origem.
		USceneComponent* TestActorRoot = NewObject<USceneComponent>(TestActor, TEXT("TestActorRoot"));
		TestActor->SetRootComponent(TestActorRoot);
		TestActorRoot->RegisterComponent();
	});

	AfterEach([this]()
	{
		if (TestActor)
		{
			TestActor->Destroy();
			TestActor = nullptr;
		}

		if (PortalA)
		{
			PortalA->Destroy();
			PortalA = nullptr;
		}

		if (PortalB)
		{
			PortalB->Destroy();
			PortalB = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should register, find, and update portal availability state", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// 1. Busca por Tag
		ASBPortalActor* FoundA = PortalSubsystem->FindPortalByTag(Tags.Portal_Type_Gateway);
		ASBPortalActor* FoundB = PortalSubsystem->FindPortalByTag(Tags.Portal_Type_Waystone);

		TestEqual("Found portal A should match", FoundA, PortalA);
		TestEqual("Found portal B should match", FoundB, PortalB);
		TestTrue("Portal A should be available", PortalSubsystem->IsPortalAvailable(Tags.Portal_Type_Gateway));

		// 2. Tranca o portal A
		PortalSubsystem->SetPortalLocked(Tags.Portal_Type_Gateway, true);
		TestFalse("Portal A should not be available when locked", PortalSubsystem->IsPortalAvailable(Tags.Portal_Type_Gateway));

		// 3. Destranca o portal A
		PortalSubsystem->SetPortalLocked(Tags.Portal_Type_Gateway, false);
		TestTrue("Portal A should be available again", PortalSubsystem->IsPortalAvailable(Tags.Portal_Type_Gateway));
	});

	It("Should teleport actor to target destination portal in same map", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// 1. Configura o destino de A para apontar para B
		FSBPortalDestination Dest;
		Dest.TargetPortalTag = Tags.Portal_Type_Waystone;

		// 2. Executa o teleporte
		bool bSuccess = PortalSubsystem->RequestTeleport(TestActor, Dest, Tags.Portal_Type_Gateway);
		TestTrue("Teleport request should succeed", bSuccess);

		// 3. Valida a nova localização do ator (Spawn location de Portal B)
		FVector ExpectedLocation = PortalB->GetTeleportSpawnLocation();
		TestNearlyEqual("Actor X location", (float)TestActor->GetActorLocation().X, (float)ExpectedLocation.X, 1.0f);
		TestNearlyEqual("Actor Y location", (float)TestActor->GetActorLocation().Y, (float)ExpectedLocation.Y, 1.0f);
		TestNearlyEqual("Actor Z location", (float)TestActor->GetActorLocation().Z, (float)ExpectedLocation.Z, 1.0f);
	});

	It("Should block teleportation when destination portal is locked", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		// 1. Tranca o Portal B
		PortalSubsystem->SetPortalLocked(Tags.Portal_Type_Waystone, true);

		FSBPortalDestination Dest;
		Dest.TargetPortalTag = Tags.Portal_Type_Waystone;

		// 2. Tenta teleportar
		bool bSuccess = PortalSubsystem->RequestTeleport(TestActor, Dest, Tags.Portal_Type_Gateway);
		TestFalse("Teleport request should fail for locked portal", bSuccess);

		// 3. Verifica que o ator não se moveu
		TestEqual("Actor should remain at origin", TestActor->GetActorLocation(), FVector::ZeroVector);
	});
}
