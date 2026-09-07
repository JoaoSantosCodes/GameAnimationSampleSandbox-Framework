// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/SBParkourComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBParkourTestsSpec, "Sandbox.Character.ParkourLocomotion", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	AActor* CharacterActor;
	USBParkourComponent* ParkourComp;
	USBStateComponent* StateComp;
END_DEFINE_SPEC(FSBParkourTestsSpec)

void FSBParkourTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CharacterActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(CharacterActor, TEXT("Root"));
		CharacterActor->SetRootComponent(Root);
		Root->RegisterComponent();

		StateComp = NewObject<USBStateComponent>(CharacterActor, TEXT("StateComp"));
		CharacterActor->AddOwnedComponent(StateComp);

		ParkourComp = NewObject<USBParkourComponent>(CharacterActor, TEXT("ParkourComp"));
		CharacterActor->AddOwnedComponent(ParkourComp);

		ISBComponentInterface::Execute_OnInitialize(StateComp);
		ISBComponentInterface::Execute_OnInitialize(ParkourComp);
	});

	AfterEach([this]()
	{
		if (CharacterActor)
		{
			CharacterActor->Destroy();
			CharacterActor = nullptr;
		}

		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
			TestWorld = nullptr;
		}
	});

	It("Should classify low obstacle as Vault and high obstacle as Mantle", [this]()
	{
		// Low obstacle: Height 75.0f, Depth 30.0f -> Vault
		FSBParkourObstacleData VaultData = ParkourComp->DetectObstacle(FVector(100.0f, 0.0f, 0.0f), FVector(-1.0f, 0.0f, 0.0f), 75.0f, 30.0f);
		TestEqual("Recommended action is Vault", (int32)VaultData.RecommendedAction, (int32)ESBParkourActionType::Vault);

		// High obstacle: Height 170.0f, Depth 100.0f -> Mantle
		FSBParkourObstacleData MantleData = ParkourComp->DetectObstacle(FVector(100.0f, 0.0f, 0.0f), FVector(-1.0f, 0.0f, 0.0f), 170.0f, 100.0f);
		TestEqual("Recommended action is Mantle", (int32)MantleData.RecommendedAction, (int32)ESBParkourActionType::Mantle);

		// Too high obstacle: Height 350.0f -> None
		FSBParkourObstacleData ImpossibleData = ParkourComp->DetectObstacle(FVector(100.0f, 0.0f, 0.0f), FVector(-1.0f, 0.0f, 0.0f), 350.0f, 50.0f);
		TestEqual("Recommended action is None", (int32)ImpossibleData.RecommendedAction, (int32)ESBParkourActionType::None);
	});

	It("Should start Vault action and apply parkour state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bActionFired = false;
		ESBParkourActionType FiredType = ESBParkourActionType::None;
		ParkourComp->OnParkourActionStarted.AddLambda([&bActionFired, &FiredType](ESBParkourActionType Type, const FSBParkourObstacleData& Data)
		{
			bActionFired = true;
			FiredType = Type;
		});

		FSBParkourObstacleData Obs = ParkourComp->DetectObstacle(FVector(100.0f, 0.0f, 0.0f), FVector(-1.0f, 0.0f, 0.0f), 80.0f, 40.0f);
		bool bSuccess = ParkourComp->StartParkourAction(Obs);

		TestTrue("StartParkourAction succeeded", bSuccess);
		TestTrue("IsPerformingParkour is true", ParkourComp->IsPerformingParkour());
		TestEqual("Action is Vault", (int32)ParkourComp->GetCurrentParkourAction(), (int32)ESBParkourActionType::Vault);
		TestTrue("Has ParkourActive tag", StateComp->HasTag(Tags.State_Movement_ParkourActive));
		TestTrue("Has Vaulting tag", StateComp->HasTag(Tags.State_Movement_Vaulting));
		TestFalse("Does not have Mantling tag", StateComp->HasTag(Tags.State_Movement_Mantling));
		TestTrue("Delegate fired", bActionFired);
		TestEqual("Delegate type is Vault", (int32)FiredType, (int32)ESBParkourActionType::Vault);
	});

	It("Should start Mantle action and apply mantle state tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		FSBParkourObstacleData Obs = ParkourComp->DetectObstacle(FVector(100.0f, 0.0f, 0.0f), FVector(-1.0f, 0.0f, 0.0f), 180.0f, 50.0f);
		ParkourComp->StartParkourAction(Obs);

		TestTrue("IsPerformingParkour is true", ParkourComp->IsPerformingParkour());
		TestEqual("Action is Mantle", (int32)ParkourComp->GetCurrentParkourAction(), (int32)ESBParkourActionType::Mantle);
		TestTrue("Has ParkourActive tag", StateComp->HasTag(Tags.State_Movement_ParkourActive));
		TestTrue("Has Mantling tag", StateComp->HasTag(Tags.State_Movement_Mantling));
		TestFalse("Does not have Vaulting tag", StateComp->HasTag(Tags.State_Movement_Vaulting));
	});

	It("Should complete parkour action and clear all parkour tags", [this]()
	{
		const FSBGameplayTags& Tags = FSBGameplayTags::Get();

		bool bCompletedFired = false;
		ESBParkourActionType CompletedType = ESBParkourActionType::None;
		ParkourComp->OnParkourActionCompleted.AddLambda([&bCompletedFired, &CompletedType](ESBParkourActionType Type)
		{
			bCompletedFired = true;
			CompletedType = Type;
		});

		FSBParkourObstacleData Obs = ParkourComp->DetectObstacle(FVector(100.0f, 0.0f, 0.0f), FVector(-1.0f, 0.0f, 0.0f), 80.0f, 40.0f);
		ParkourComp->StartParkourAction(Obs);

		TestTrue("Performing parkour before complete", ParkourComp->IsPerformingParkour());

		ParkourComp->CompleteParkourAction();

		TestFalse("Not performing parkour after complete", ParkourComp->IsPerformingParkour());
		TestEqual("Current action is None", (int32)ParkourComp->GetCurrentParkourAction(), (int32)ESBParkourActionType::None);
		TestFalse("ParkourActive tag removed", StateComp->HasTag(Tags.State_Movement_ParkourActive));
		TestFalse("Vaulting tag removed", StateComp->HasTag(Tags.State_Movement_Vaulting));
		TestTrue("Completed delegate fired", bCompletedFired);
		TestEqual("Completed action is Vault", (int32)CompletedType, (int32)ESBParkourActionType::Vault);
	});
}
