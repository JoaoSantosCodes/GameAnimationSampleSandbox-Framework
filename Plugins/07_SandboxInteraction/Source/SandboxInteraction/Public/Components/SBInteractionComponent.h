#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SBComponentInterface.h"
#include "Interfaces/SBInteractableInterface.h"
#include "Subsystems/SBEventPayloads.h"
#include "SBInteractionComponent.generated.h"

class USBStateComponent;

USTRUCT(BlueprintType)
struct FSBActiveInteractionState
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY()
	float StartTime = 0.0f;

	UPROPERTY()
	bool bIsHold = false;
};

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINTERACTION_API USBInteractionComponent : public UGameFrameworkComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBInteractionComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// API de Entrada
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Interaction")
	void StartInteract();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Interaction")
	void StopInteract();

	// RPCs de Rede
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartInteract(AActor* Target);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCompleteInteract(AActor* Target);

	UFUNCTION(Client, Reliable)
	void ClientCancelInteraction();

	// Getters
	UFUNCTION(BlueprintPure, Category = "Sandbox|Interaction")
	AActor* GetCurrentInteractableActor() const { return CurrentInteractableActor; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Interaction")
	bool IsHoldingInteraction() const { return bIsHoldingInteraction; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Interaction")
	float GetHoldProgressPercent() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Interaction")
	float InteractionRange = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Interaction")
	bool bUseLineTrace = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Interaction")
	float TraceRadius = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentInteractableActor = nullptr;

	UPROPERTY(Transient)
	bool bIsHoldingInteraction = false;

	UPROPERTY(Transient)
	float HoldProgressTime = 0.0f;

	UPROPERTY(Transient)
	float CurrentHoldDuration = 0.0f;

	UPROPERTY(Transient)
	float ProgressBroadcastAccumulator = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<USBStateComponent> CachedStateComponent = nullptr;

	// Estado do Lock no Servidor
	UPROPERTY(Transient)
	FSBActiveInteractionState ActiveServerInteraction;

	virtual void ScanForInteractables();
	class USBEventSubsystem* GetEventSubsystem() const;

protected:
	bool Target_CanInteract(AActor* Target) const;
	void Target_Interact(AActor* Target);
	FText Target_GetInteractionPrompt(AActor* Target) const;
	float Target_GetInteractionDuration(AActor* Target) const;
	bool Target_IsInteractionLocked(AActor* Target) const;
	void Target_LockInteraction(AActor* Target);
	void Target_UnlockInteraction(AActor* Target);

private:
	template<typename TReturn, typename TExecuteCallable, typename TNativeCallable>
	TReturn RouteInterfaceCall(AActor* Target, TExecuteCallable ExecuteFn, TNativeCallable NativeFn, TReturn DefaultValue) const
	{
		if (!Target || !Target->GetClass()->ImplementsInterface(USBInteractableInterface::StaticClass()))
		{
			return DefaultValue;
		}

		if (Target->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
		{
			return ExecuteFn();
		}

		if (ISBInteractableInterface* Interactable = Cast<ISBInteractableInterface>(Target))
		{
			return NativeFn(Interactable);
		}

		return DefaultValue;
	}

	template<typename TExecuteCallable, typename TNativeCallable>
	void RouteInterfaceCallVoid(AActor* Target, TExecuteCallable ExecuteFn, TNativeCallable NativeFn) const
	{
		if (!Target || !Target->GetClass()->ImplementsInterface(USBInteractableInterface::StaticClass()))
		{
			return;
		}

		if (Target->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
		{
			ExecuteFn();
			return;
		}

		if (ISBInteractableInterface* Interactable = Cast<ISBInteractableInterface>(Target))
		{
			NativeFn(Interactable);
		}
	}
};
