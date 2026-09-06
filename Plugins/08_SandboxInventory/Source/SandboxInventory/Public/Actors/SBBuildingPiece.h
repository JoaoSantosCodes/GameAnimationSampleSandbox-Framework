#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "SBBuildingPiece.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS(BlueprintType, Blueprintable)
class SANDBOXINVENTORY_API ASBBuildingPiece : public AActor
{
	GENERATED_BODY()

public:
	ASBBuildingPiece();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building")
	FGameplayTag BuildingPieceTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building")
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Building")
	float Health;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building")
	FString OwnerPlayerName;

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetPreviewMode(bool bIsPreview);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Building")
	void ServerTakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Building")
	UBoxComponent* GetCollisionBox() const { return CollisionBox; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnRep_Health();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;
};
