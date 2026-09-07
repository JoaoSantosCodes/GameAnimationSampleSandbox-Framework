// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SBInteractableInterface.h"
#include "SBContainerChest.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class USBInventoryComponent;

UCLASS()
class SANDBOXINVENTORY_API ASBContainerChest : public AActor, public ISBInteractableInterface
{
	GENERATED_BODY()

public:
	ASBContainerChest();

	virtual void Tick(float DeltaTime) override;

	// ISBInteractableInterface
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual float GetInteractionDuration_Implementation(AActor* Interactor) const override { return 0.0f; }
	virtual bool IsInteractionLocked_Implementation(AActor* Interactor) const override { return false; }
	virtual void LockInteraction_Implementation(AActor* Interactor) override {}
	virtual void UnlockInteraction_Implementation(AActor* Interactor) override {}

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Inventory")
	void StopInteracting(AActor* Interactor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USBInventoryComponent> InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Inventory")
	float MaxInteractionDistance;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

private:
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> ActiveInteractors;

	void RemoveInteractorAt(int32 Index);
};
