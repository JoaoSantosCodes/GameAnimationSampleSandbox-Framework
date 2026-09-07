// Copyright 2026 João Santos. All Rights Reserved.
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SBInteractableInterface.h"
#include "GameplayTagContainer.h"
#include "SBCraftingStation.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class USBCraftingRecipeDataAsset;

/**
 * Ator que representa uma bancada física de Crafting interativa.
 */
UCLASS()
class SANDBOXINVENTORY_API ASBCraftingStation : public AActor, public ISBInteractableInterface
{
	GENERATED_BODY()

public:
	ASBCraftingStation();

	virtual void Tick(float DeltaTime) override;

	// ISBInteractableInterface
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual float GetInteractionDuration_Implementation(AActor* Interactor) const override { return 0.0f; }
	virtual bool IsInteractionLocked_Implementation(AActor* Interactor) const override { return false; }
	virtual void LockInteraction_Implementation(AActor* Interactor) override {}
	virtual void UnlockInteraction_Implementation(AActor* Interactor) override {}

	/** Remove manualmente a tag do interator (ex: ao fechar a UI) */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Crafting")
	void StopInteracting(AActor* Interactor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Crafting")
	FGameplayTag StationTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Crafting")
	TArray<USBCraftingRecipeDataAsset*> SupportedRecipes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Crafting")
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
