// Copyright 2026 João Santos. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Components/SBInventoryComponent.h"
#include "Components/SBBuildingComponent.h"
#include "Components/BoxComponent.h"
#include "Actors/SBBuildingPiece.h"
#include "Items/SBItemDefinition.h"
#include "Items/SBItemInstance.h"
#include "Items/SBItemFragment_Placeable.h"
#include "SBGameplayTags.h"

BEGIN_DEFINE_SPEC(FSBBuildingTestsSpec, "Sandbox.Inventory.Building", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)
	UWorld* TestWorld;
	ACharacter* PlayerCharacter;
	USBInventoryComponent* InventoryComponent;
	USBBuildingComponent* BuildingComponent;
	USBItemDefinition* PlaceableItemDef;
	USBItemInstance* PlaceableItemInstance;
	TSubclassOf<ASBBuildingPiece> DefaultPieceClass;
END_DEFINE_SPEC(FSBBuildingTestsSpec)

void FSBBuildingTestsSpec::Define()
{
	BeforeEach([this]()
	{
		TestWorld = UWorld::CreateWorld(EWorldType::Game, false, TEXT("TestWorld"));

		// Associa o GameInstance global do Editor/Runner ao mundo de teste para que subsystems funcionem se necessário
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

		FSBGameplayTags::InitializeNativeTags();

		// 1. Cria Personagem do Jogador
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = TEXT("BuildingTestPlayerPawn");
		PlayerCharacter = TestWorld->SpawnActor<ACharacter>(ACharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		InventoryComponent = NewObject<USBInventoryComponent>(PlayerCharacter, TEXT("InventoryComponent"));
		InventoryComponent->RegisterComponent();

		BuildingComponent = NewObject<USBBuildingComponent>(PlayerCharacter, TEXT("BuildingComponent"));
		BuildingComponent->RegisterComponent();

		PlayerCharacter->DispatchBeginPlay();

		// 2. Cria Item Definition e Fragment de teste
		PlaceableItemDef = NewObject<USBItemDefinition>(TestWorld, TEXT("FloorItemDef"));
		const_cast<FText&>(PlaceableItemDef->DisplayName) = FText::FromString(TEXT("Plataforma de Madeira"));
		const_cast<int32&>(PlaceableItemDef->MaxStackCount) = 10;

		USBItemFragment_Placeable* PlaceableFrag = NewObject<USBItemFragment_Placeable>(PlaceableItemDef);
		PlaceableFrag->BuildingPieceClass = ASBBuildingPiece::StaticClass();
		PlaceableItemDef->Fragments.Add(PlaceableFrag);

		DefaultPieceClass = ASBBuildingPiece::StaticClass();

		// Adiciona 1 unidade ao inventário do player
		PlaceableItemInstance = InventoryComponent->ServerAddItem(PlaceableItemDef, 1);
	});

	AfterEach([this]()
	{
		if (PlayerCharacter)
		{
			PlayerCharacter->Destroy();
		}
		if (TestWorld)
		{
			TestWorld->DestroyWorld(false);
		}
	});

	Describe("Preview and Snapping Mechanism", [this]()
	{
		It("Should spawn local Preview Actor when StartPlacement is invoked and destroy it on StopPlacement", [this]()
		{
			TestNull("PreviewActor deve ser nulo inicialmente", BuildingComponent->GetPreviewActor());

			BuildingComponent->StartPlacement(DefaultPieceClass, PlaceableItemInstance);

			ASBBuildingPiece* Preview = BuildingComponent->GetPreviewActor();
			TestNotNull("PreviewActor deve ter sido instanciado", Preview);
			TestEqual("PreviewActor deve pertencer à classe correta", Preview->GetClass(), Cast<UClass>(*DefaultPieceClass));

			BuildingComponent->StopPlacement();
			TestNull("PreviewActor deve ser nulo após StopPlacement", BuildingComponent->GetPreviewActor());
		});

		It("Should snap preview coordinates precisely on 200x200x100 increments", [this]()
		{
			BuildingComponent->StartPlacement(DefaultPieceClass, PlaceableItemInstance);
			ASBBuildingPiece* Preview = BuildingComponent->GetPreviewActor();
			TestNotNull("PreviewActor deve existir", Preview);

			// Testa snap em várias direções
			BuildingComponent->UpdatePreview(FVector(110.0f, 90.0f, 40.0f), FRotator::ZeroRotator);
			TestEqual("Coordenada X deve dar snap para 200", Preview->GetActorLocation().X, 200.0);
			// Let's test with exact math values:
			// Aim X = 110 -> 110/200 = 0.55 -> Round = 1.0f -> SnappedX = 200.0.
			// Aim Y = 90 -> 90/200 = 0.45 -> Round = 0.0f -> SnappedY = 0.0.
			// Aim Z = 40 -> 40/100 = 0.40 -> Round = 0.0f -> SnappedZ = 0.0.
			
			BuildingComponent->UpdatePreview(FVector(290.0f, -110.0f, 160.0f), FRotator::ZeroRotator);
			TestEqual("Coordenada X deve dar snap para 200 (290 / 200 -> 1.45 -> 1.0)", Preview->GetActorLocation().X, 200.0);
			TestEqual("Coordenada Y deve dar snap para -200 (-110 / 200 -> -0.55 -> -1.0)", Preview->GetActorLocation().Y, -200.0);
			TestEqual("Coordenada Z deve dar snap para 200 (160 / 100 -> 1.6 -> 2.0)", Preview->GetActorLocation().Z, 200.0);

			BuildingComponent->StopPlacement();
		});
	});

	Describe("Server Placement Validations", [this]()
	{
		It("Should successfully place building piece and consume inventory item on legitimate request", [this]()
		{
			// Transform válido a 200 unidades à frente
			FTransform TargetTransform(FRotator::ZeroRotator, FVector(200.0f, 0.0f, 0.0f));

			BuildingComponent->StartPlacement(DefaultPieceClass, PlaceableItemInstance);
			BuildingComponent->UpdatePreview(TargetTransform.GetLocation(), TargetTransform.Rotator());
			
			BuildingComponent->RequestPlaceActivePiece();

			// Verifica se o item foi consumido (Quantidade total deve ser 0)
			TestEqual("Item deve ter sido removido do inventário", InventoryComponent->GetTotalItemQuantity(PlaceableItemDef), 0);

			// Verifica se a peça física replicada foi spawnada no mundo
			TArray<AActor*> SpawnedPieces;
			for (TActorIterator<ASBBuildingPiece> It(TestWorld); It; ++It)
			{
				if (*It != BuildingComponent->GetPreviewActor())
				{
					SpawnedPieces.Add(*It);
				}
			}

			TestEqual("Deve haver exatamente 1 peça física oficial no mundo", SpawnedPieces.Num(), 1);
			if (SpawnedPieces.Num() > 0)
			{
				ASBBuildingPiece* PlacedPiece = Cast<ASBBuildingPiece>(SpawnedPieces[0]);
				TestNotNull("Peça instanciada não deve ser nula", PlacedPiece);
				TestEqual("Dono da peça deve ser o player", PlacedPiece->OwnerPlayerName, PlayerCharacter->GetName());
				TestEqual("Posição da peça deve ser a da transform solicitada", PlacedPiece->GetActorLocation(), FVector(200.0f, 0.0f, 0.0f));
			}

			BuildingComponent->StopPlacement();
		});

		It("Should reject placement if target transform is too far (Proximity check)", [this]()
		{
			// Posição a 1000 unidades (Limite é 600)
			FTransform FarTransform(FRotator::ZeroRotator, FVector(1000.0f, 0.0f, 0.0f));

			BuildingComponent->StartPlacement(DefaultPieceClass, PlaceableItemInstance);
			BuildingComponent->UpdatePreview(FarTransform.GetLocation(), FarTransform.Rotator());

			BuildingComponent->RequestPlaceActivePiece();

			// Item NÃO deve ser consumido
			TestEqual("Item NÃO deve ser removido se a transação falhar por distância", InventoryComponent->GetTotalItemQuantity(PlaceableItemDef), 1);

			// Nenhuma peça física oficial deve ser spawnada
			TArray<AActor*> SpawnedPieces;
			for (TActorIterator<ASBBuildingPiece> It(TestWorld); It; ++It)
			{
				if (*It != BuildingComponent->GetPreviewActor())
				{
					SpawnedPieces.Add(*It);
				}
			}
			TestEqual("Nenhuma peça física deve ter sido spawnada além da distância permitida", SpawnedPieces.Num(), 0);

			BuildingComponent->StopPlacement();
		});

		It("Should reject placement if target location overlaps static geometry (Overlap check)", [this]()
		{
			// 1. Spawna um ator estático que causará overlap na coordenada 200, 0, 0
			FActorSpawnParameters BlockerParams;
			BlockerParams.Name = TEXT("StaticBlockerActor");
			AActor* BlockerActor = TestWorld->SpawnActor<AActor>(AActor::StaticClass(), FVector(200.0f, 0.0f, 0.0f), FRotator::ZeroRotator, BlockerParams);
			UBoxComponent* BlockerBox = NewObject<UBoxComponent>(BlockerActor, TEXT("BlockerBox"));
			BlockerActor->SetRootComponent(BlockerBox);
			BlockerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
			BlockerBox->SetCollisionProfileName(TEXT("BlockAll"));
			BlockerBox->SetCollisionObjectType(ECC_WorldStatic);
			BlockerBox->RegisterComponent();
			BlockerActor->SetActorLocation(FVector(200.0f, 0.0f, 0.0f));
			BlockerActor->DispatchBeginPlay();

			// 2. Tenta posicionar a peça no exato local do Blocker
			FTransform TargetTransform(FRotator::ZeroRotator, FVector(200.0f, 0.0f, 0.0f));

			BuildingComponent->StartPlacement(DefaultPieceClass, PlaceableItemInstance);
			BuildingComponent->UpdatePreview(TargetTransform.GetLocation(), TargetTransform.Rotator());

			BuildingComponent->RequestPlaceActivePiece();

			// Item NÃO deve ser consumido devido ao overlap
			TestEqual("Item NÃO deve ser removido se houver colisão de overlap", InventoryComponent->GetTotalItemQuantity(PlaceableItemDef), 1);

			// Nenhuma peça física deve ser spawnada
			TArray<AActor*> SpawnedPieces;
			for (TActorIterator<ASBBuildingPiece> It(TestWorld); It; ++It)
			{
				if (*It != BuildingComponent->GetPreviewActor())
				{
					SpawnedPieces.Add(*It);
				}
			}
			TestEqual("Nenhuma peça física deve ter sido spawnada em colisão", SpawnedPieces.Num(), 0);

			BuildingComponent->StopPlacement();
			BlockerActor->Destroy();
		});

		It("Should reject placement if player does not own the placeable item in inventory", [this]()
		{
			// Remove o item do inventário do player
			InventoryComponent->ServerRemoveItem(PlaceableItemInstance, 1);

			FTransform TargetTransform(FRotator::ZeroRotator, FVector(200.0f, 0.0f, 0.0f));
			BuildingComponent->StartPlacement(DefaultPieceClass, PlaceableItemInstance);
			BuildingComponent->UpdatePreview(TargetTransform.GetLocation(), TargetTransform.Rotator());

			BuildingComponent->RequestPlaceActivePiece();

			// Nenhuma peça física deve ser spawnada
			TArray<AActor*> SpawnedPieces;
			for (TActorIterator<ASBBuildingPiece> It(TestWorld); It; ++It)
			{
				if (*It != BuildingComponent->GetPreviewActor())
				{
					SpawnedPieces.Add(*It);
				}
			}
			TestEqual("Nenhuma peça física deve ter sido spawnada se o item não é possuído", SpawnedPieces.Num(), 0);

			BuildingComponent->StopPlacement();
		});
	});

	Describe("Structural Health and Damage", [this]()
	{
		It("Should take damage and destroy structural piece when health hits 0", [this]()
		{
			FActorSpawnParameters SpawnParams;
			ASBBuildingPiece* PlacedPiece = TestWorld->SpawnActor<ASBBuildingPiece>(DefaultPieceClass, FVector(200.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
			TestNotNull("Peça física instalada com sucesso", PlacedPiece);
			TestEqual("HP inicial deve ser MaxHealth", PlacedPiece->Health, PlacedPiece->MaxHealth);

			// Aplica dano parcial
			PlacedPiece->ServerTakeDamage(40.0f);
			TestEqual("HP deve ter sido reduzido para 60", PlacedPiece->Health, 60.0f);

			// Aplica dano letal
			PlacedPiece->ServerTakeDamage(60.0f);
			
			// A peça deve ter sido destruída e não deve ser encontrada no mundo
			TArray<AActor*> FoundPieces;
			for (TActorIterator<ASBBuildingPiece> It(TestWorld); It; ++It)
			{
				FoundPieces.Add(*It);
			}
			TestEqual("Peça deve ter sido destruída e removida do mundo", FoundPieces.Num(), 0);
		});
	});
}
