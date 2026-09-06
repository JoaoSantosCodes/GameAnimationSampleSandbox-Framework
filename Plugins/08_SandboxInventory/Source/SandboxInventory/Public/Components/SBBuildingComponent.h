#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayTagContainer.h"
#include "Actors/SBBuildingPiece.h"
#include "Items/SBItemInstance.h"
#include "SBBuildingComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBBuildingComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	USBBuildingComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building")
	float MaxBuildDistance = 600.0f;

	UFUNCTION(BlueprintCallable, Category = "Building")
	void StartPlacement(TSubclassOf<ASBBuildingPiece> PieceClass, USBItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Building")
	void StopPlacement();

	UFUNCTION(BlueprintCallable, Category = "Building")
	void UpdatePreview(const FVector& AimLocation, const FRotator& AimRotation);

	UFUNCTION(BlueprintCallable, Category = "Building")
	void RequestPlaceActivePiece();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Building")
	void ServerPlaceBuildingPiece(TSubclassOf<ASBBuildingPiece> PieceClass, FTransform TargetTransform, USBItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category = "Building")
	ASBBuildingPiece* GetPreviewActor() const { return PreviewActor; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY()
	TSubclassOf<ASBBuildingPiece> ActivePieceClass = nullptr;

	UPROPERTY()
	TObjectPtr<ASBBuildingPiece> PreviewActor = nullptr;

	UPROPERTY()
	TObjectPtr<USBItemInstance> ActiveItemInstance = nullptr;
};
