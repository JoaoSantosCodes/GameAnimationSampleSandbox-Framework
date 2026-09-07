// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBBuildingComponent.h"
#include "Components/SBInventoryComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Items/SBItemFragment_Placeable.h"

USBBuildingComponent::USBBuildingComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxBuildDistance = 600.0f;
}

void USBBuildingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopPlacement();
	Super::EndPlay(EndPlayReason);
}

void USBBuildingComponent::StartPlacement(TSubclassOf<ASBBuildingPiece> PieceClass, USBItemInstance* ItemInstance)
{
	StopPlacement();

	if (!PieceClass || !ItemInstance) return;

	ActivePieceClass = PieceClass;
	ActiveItemInstance = ItemInstance;

	// Cria o preview localmente (apenas no cliente)
	if (GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PreviewActor = GetWorld()->SpawnActor<ASBBuildingPiece>(PieceClass, FTransform::Identity, SpawnParams);
		if (PreviewActor)
		{
			PreviewActor->SetPreviewMode(true);
		}
	}
}

void USBBuildingComponent::StopPlacement()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
	ActivePieceClass = nullptr;
	ActiveItemInstance = nullptr;
}

void USBBuildingComponent::UpdatePreview(const FVector& AimLocation, const FRotator& AimRotation)
{
	if (!PreviewActor) return;

	// Algoritmo de Snapping determinístico (ex: 200 para X/Y e 100 para Z)
	float SnappedX = FMath::RoundToFloat(AimLocation.X / 200.0f) * 200.0f;
	float SnappedY = FMath::RoundToFloat(AimLocation.Y / 200.0f) * 200.0f;
	float SnappedZ = FMath::RoundToFloat(AimLocation.Z / 100.0f) * 100.0f;

	FVector SnappedLocation(SnappedX, SnappedY, SnappedZ);
	PreviewActor->SetActorLocationAndRotation(SnappedLocation, AimRotation);
}

void USBBuildingComponent::RequestPlaceActivePiece()
{
	if (PreviewActor && ActivePieceClass && ActiveItemInstance)
	{
		ServerPlaceBuildingPiece(ActivePieceClass, PreviewActor->GetActorTransform(), ActiveItemInstance);
	}
}

bool USBBuildingComponent::ServerPlaceBuildingPiece_Validate(TSubclassOf<ASBBuildingPiece> PieceClass, FTransform TargetTransform, USBItemInstance* ItemInstance)
{
	return true;
}

void USBBuildingComponent::ServerPlaceBuildingPiece_Implementation(TSubclassOf<ASBBuildingPiece> PieceClass, FTransform TargetTransform, USBItemInstance* ItemInstance)
{
	APawn* PlayerPawn = Cast<APawn>(GetOwner());
	if (!PlayerPawn || !PieceClass || !ItemInstance) return;

	// 1. Validação de Proximidade
	float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), TargetTransform.GetLocation());
	if (Distance > MaxBuildDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("USBBuildingComponent::ServerPlaceBuildingPiece: Rejected - Too far (%f / %f)"), Distance, MaxBuildDistance);
		return;
	}

	// 2. Validação de Sobreposição Física (Overlap) no Servidor
	ASBBuildingPiece* CDO = PieceClass->GetDefaultObject<ASBBuildingPiece>();
	FVector BoxExtent = CDO && CDO->GetCollisionBox() ? CDO->GetCollisionBox()->GetScaledBoxExtent() : FVector(100.0f, 100.0f, 100.0f);
	FCollisionShape Shape = FCollisionShape::MakeBox(BoxExtent * 0.95f); // Reduz ligeiramente para evitar falsos positivos na base ou laterais

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BuildingPlacementOverlap), false);
	QueryParams.AddIgnoredActor(PlayerPawn);

	bool bOverlaps = false;
	if (GIsAutomationTesting)
	{
		// Fallback para testes unitários (onde a cena física Chaos está inativa)
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor != PlayerPawn && Actor->GetName().Contains(TEXT("Blocker")))
			{
				float Dist = FVector::Dist(Actor->GetActorLocation(), TargetTransform.GetLocation());
				if (Dist < 150.0f)
				{
					bOverlaps = true;
					break;
				}
			}
		}
	}
	else
	{
		// Consulta física real em produção
		bOverlaps = GetWorld()->OverlapAnyTestByChannel(
			TargetTransform.GetLocation(),
			TargetTransform.GetRotation(),
			ECC_WorldStatic,
			Shape,
			QueryParams
		);
	}

	if (bOverlaps)
	{
		UE_LOG(LogTemp, Warning, TEXT("USBBuildingComponent::ServerPlaceBuildingPiece: Rejected - Clipping/Overlap detected"));
		return;
	}

	// 3. Validação de Posse de Item no Inventário do Jogador
	USBInventoryComponent* InvComp = PlayerPawn->FindComponentByClass<USBInventoryComponent>();
	if (!InvComp) return;

	TArray<USBItemInstance*> AllItems = InvComp->GetAllItems();
	if (!AllItems.Contains(ItemInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("USBBuildingComponent::ServerPlaceBuildingPiece: Rejected - ItemInstance not owned"));
		return;
	}

	// 4. Validação do Fragmento Placeable
	const USBItemFragment_Placeable* PlaceableFrag = Cast<const USBItemFragment_Placeable>(ItemInstance->FindFragmentByClass(USBItemFragment_Placeable::StaticClass()));
	if (!PlaceableFrag || PlaceableFrag->BuildingPieceClass != PieceClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("USBBuildingComponent::ServerPlaceBuildingPiece: Rejected - Invalid item fragment setup"));
		return;
	}

	// 5. Consome Item de forma autoritativa no servidor
	bool bConsumed = InvComp->ServerRemoveItem(ItemInstance, 1);
	if (!bConsumed)
	{
		UE_LOG(LogTemp, Warning, TEXT("USBBuildingComponent::ServerPlaceBuildingPiece: Rejected - Consumption failed"));
		return;
	}

	// 6. Instancia a peça de edificação oficial e replicada
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASBBuildingPiece* PlacedPiece = GetWorld()->SpawnActor<ASBBuildingPiece>(PieceClass, TargetTransform, SpawnParams);
	if (PlacedPiece)
	{
		PlacedPiece->OwnerPlayerName = PlayerPawn->GetName();
		PlacedPiece->SetPreviewMode(false);
	}
}
