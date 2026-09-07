// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SBInteractableInterface.h"
#include "Items/SBItemDefinition.h"
#include "SBPhysicalLootDrop.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class SANDBOXINVENTORY_API ASBPhysicalLootDrop : public AActor, public ISBInteractableInterface
{
	GENERATED_BODY()

public:
	ASBPhysicalLootDrop();

	// ISBInteractableInterface
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual float GetInteractionDuration_Implementation(AActor* Interactor) const override { return InteractionDuration; }
	virtual bool IsInteractionLocked_Implementation(AActor* Interactor) const override { return bIsLocked; }
	virtual void LockInteraction_Implementation(AActor* Interactor) override { bIsLocked = true; }
	virtual void UnlockInteraction_Implementation(AActor* Interactor) override { bIsLocked = false; }

	// Inicialização autoritativa do drop no servidor
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sandbox|Loot")
	void InitializeLoot(USBItemDefinition* InItemDef, int32 InStackCount);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Loot")
	USBItemDefinition* GetItemDefinition() const { return ItemDefinition; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Loot")
	int32 GetStackCount() const { return StackCount; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Loot")
	FGameplayTag GetRarityTag() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Loot")
	FLinearColor GetRarityColor() const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Loot")
	virtual void UpdateVisuals();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(ReplicatedUsing = OnRep_ItemDefinition, EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<USBItemDefinition> ItemDefinition;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 StackCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	float InteractionDuration = 0.0f;

	UPROPERTY(Transient)
	bool bIsLocked = false;

	UFUNCTION()
	void OnRep_ItemDefinition();
};
