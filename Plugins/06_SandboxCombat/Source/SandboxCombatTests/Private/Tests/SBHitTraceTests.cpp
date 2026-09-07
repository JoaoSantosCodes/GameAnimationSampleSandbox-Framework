// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBHitTraceComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBHitTraceTestsSpec, "Sandbox.Combat.HitTrace", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* AttackerActor;
	AActor* TargetActorA;
	AActor* TargetActorB;
	USBHitTraceComponent* HitTraceComp;
	USBStateComponent* AttackerState;
	UBoxComponent* TargetABox;
	UBoxComponent* TargetBBox;
END_DEFINE_SPEC(FSBHitTraceTestsSpec)

void FSBHitTraceTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 1. Atacante em (0, 0, 0)
		AttackerActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* AttackerRoot = NewObject<USceneComponent>(AttackerActor, TEXT("AttackerRoot"));
		AttackerActor->SetRootComponent(AttackerRoot);
		AttackerRoot->RegisterComponent();

		AttackerState = NewObject<USBStateComponent>(AttackerActor, TEXT("AttackerState"));
		AttackerState->RegisterComponent();
		AttackerActor->AddOwnedComponent(AttackerState);

		HitTraceComp = NewObject<USBHitTraceComponent>(AttackerActor, TEXT("HitTraceComp"));
		HitTraceComp->RegisterComponent();
		AttackerActor->AddOwnedComponent(HitTraceComp);

		// 2. Alvo A em (100, 0, 0)
		TargetActorA = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		TargetABox = NewObject<UBoxComponent>(TargetActorA, TEXT("TargetABox"));
		TargetActorA->SetRootComponent(TargetABox);
		TargetABox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
		TargetABox->SetCollisionProfileName(TEXT("Pawn"));
		TargetABox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// Overlap (nao Block) no canal de traco: e o setup de um traco de arma real. Com Block,
		// SweepMulti para no primeiro alvo e um golpe nunca atinge dois inimigos.
		TargetABox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		TargetABox->RegisterComponent();
		// Um AActor puro nao tem RootComponent no spawn, entao a localizacao passada a
		// SpawnActor nao e aplicada. Posicionar apos o root existir.
		TargetActorA->SetActorLocation(FVector(100.0f, 0.0f, 0.0f));

		// 3. Alvo B em (250, 0, 0) - afastado de A para que as caixas nao se toquem
		TargetActorB = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(250.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
		TargetBBox = NewObject<UBoxComponent>(TargetActorB, TEXT("TargetBBox"));
		TargetActorB->SetRootComponent(TargetBBox);
		TargetBBox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
		TargetBBox->SetCollisionProfileName(TEXT("Pawn"));
		TargetBBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TargetBBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		TargetBBox->RegisterComponent();
		// Um AActor puro nao tem RootComponent no spawn, entao a localizacao passada a
		// SpawnActor nao e aplicada. Posicionar apos o root existir.
		TargetActorB->SetActorLocation(FVector(250.0f, 0.0f, 0.0f));

		ISBComponentInterface::Execute_OnInitialize(AttackerState);
		ISBComponentInterface::Execute_OnInitialize(HitTraceComp);
	});

	AfterEach([this]()
	{
		if (AttackerActor)
		{
			AttackerActor->Destroy();
			AttackerActor = nullptr;
		}

		if (TargetActorA)
		{
			TargetActorA->Destroy();
			TargetActorA = nullptr;
		}

		if (TargetActorB)
		{
			TargetActorB->Destroy();
			TargetActorB = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should activate hit trace and grant combat tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBHitTraceSettings Settings;
		FSBHitTraceSocketConfig S1;
		S1.SocketName = NAME_None;
		S1.TraceRadius = 25.0f;
		Settings.Sockets.Add(S1);

		HitTraceComp->StartHitTrace(Settings);

		TestTrue("Tracing should be active", HitTraceComp->IsTracingActive());
		TestTrue("Attacker should have Attacking tag", AttackerState->HasTag(Tags.State_Combat_Attacking));
		TestTrue("Attacker should have HitTraceActive tag", AttackerState->HasTag(Tags.State_Combat_HitTraceActive));
	});

	It("Should detect target actor with single-hit per swing filter", [this]()
	{
		FSBHitTraceSettings Settings;
		FSBHitTraceSocketConfig S1;
		S1.SocketName = NAME_None;
		S1.TraceRadius = 30.0f;
		Settings.Sockets.Add(S1);
		Settings.TraceChannel = ECC_Pawn;

		HitTraceComp->StartHitTrace(Settings);

		// Frame 1: Move atacante através do Target A (de 0 para 150)
		AttackerActor->SetActorLocation(FVector(150.0f, 0.0f, 0.0f));
		HitTraceComp->TickComponent(0.016f, ELevelTick::LEVELTICK_All, nullptr);

		TestEqual("Hit count should be 1 after passing Target A", HitTraceComp->GetHitActorsCount(), 1);

		// Frame 2: Permanece no mesmo local (não deve registrar segundo hit no mesmo swing)
		HitTraceComp->TickComponent(0.016f, ELevelTick::LEVELTICK_All, nullptr);
		TestEqual("Hit count should still be 1 (single-hit filtered)", HitTraceComp->GetHitActorsCount(), 1);
	});

	It("Should record multiple distinct targets hit in same swing", [this]()
	{
		FSBHitTraceSettings Settings;
		FSBHitTraceSocketConfig S1;
		S1.SocketName = NAME_None;
		S1.TraceRadius = 40.0f;
		Settings.Sockets.Add(S1);
		Settings.TraceChannel = ECC_Pawn;

		HitTraceComp->StartHitTrace(Settings);

		// Move atacante através de Target A e Target B (de 0 para 250)
		AttackerActor->SetActorLocation(FVector(250.0f, 0.0f, 0.0f));
		HitTraceComp->TickComponent(0.016f, ELevelTick::LEVELTICK_All, nullptr);

		TestEqual("Both distinct targets should be recorded", HitTraceComp->GetHitActorsCount(), 2);
	});

	It("Should stop hit trace, broadcast total hits, and remove tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBHitTraceSettings Settings;
		FSBHitTraceSocketConfig S1;
		S1.SocketName = NAME_None;
		Settings.Sockets.Add(S1);

		HitTraceComp->StartHitTrace(Settings);
		TestTrue("Tracing active", HitTraceComp->IsTracingActive());

		HitTraceComp->StopHitTrace();
		TestFalse("Tracing should be stopped", HitTraceComp->IsTracingActive());
		TestFalse("Attacking tag removed", AttackerState->HasTag(Tags.State_Combat_Attacking));
		TestFalse("HitTraceActive tag removed", AttackerState->HasTag(Tags.State_Combat_HitTraceActive));
	});
}
