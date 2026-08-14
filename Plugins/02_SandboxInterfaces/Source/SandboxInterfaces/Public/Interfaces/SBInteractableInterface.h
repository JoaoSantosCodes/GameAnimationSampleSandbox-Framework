#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SBInteractableInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class USBInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class SANDBOXINTERFACES_API ISBInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Interaction")
	bool CanInteract(AActor* Interactor) const;
	virtual bool CanInteract_Implementation(AActor* Interactor) const { return true; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Interaction")
	void Interact(AActor* Interactor);
	virtual void Interact_Implementation(AActor* Interactor) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Interaction")
	FText GetInteractionPrompt(AActor* Interactor) const;
	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const { return FText::GetEmpty(); }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Interaction")
	float GetInteractionDuration(AActor* Interactor) const;
	virtual float GetInteractionDuration_Implementation(AActor* Interactor) const { return 0.0f; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Interaction")
	bool IsInteractionLocked(AActor* Interactor) const;
	virtual bool IsInteractionLocked_Implementation(AActor* Interactor) const { return false; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Interaction")
	void LockInteraction(AActor* Interactor);
	virtual void LockInteraction_Implementation(AActor* Interactor) {}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox Interaction")
	void UnlockInteraction(AActor* Interactor);
	virtual void UnlockInteraction_Implementation(AActor* Interactor) {}
};
