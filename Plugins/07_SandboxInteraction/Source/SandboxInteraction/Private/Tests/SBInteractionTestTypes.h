#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/SBInteractableInterface.h"
#include "Interfaces/SBDebugInterface.h"
#include "Components/SBInteractionComponent.h"
#include "GameplayTagContainer.h"
#include "Components/BoxComponent.h"
#include "Subsystems/SBEventSubsystem.h"
#include "SBInteractionTestTypes.generated.h"

UCLASS()
class ASBTestInteractableActor : public AActor, public ISBInteractableInterface, public ISBDebugInterface
{
	GENERATED_BODY()

public:
	ASBTestInteractableActor()
	{
		PrimaryActorTick.bCanEverTick = false;
		bReplicates = true;
		CustomPrompt = FText::FromString("TestPrompt");

		UBoxComponent* BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
		BoxCollision->SetCollisionProfileName(TEXT("BlockAll"));
		BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		BoxCollision->SetCollisionResponseToAllChannels(ECR_Block);
		BoxCollision->InitBoxExtent(FVector(50.f, 50.f, 50.f));
		RootComponent = BoxCollision;
	}

	UPROPERTY()
	float CustomDuration = 0.0f;

	UPROPERTY()
	FText CustomPrompt;

	UPROPERTY()
	bool bCanInteractFlag = true;

	UPROPERTY()
	int32 InteractCount = 0;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentLockOwner = nullptr;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override
	{
		return bCanInteractFlag;
	}

	virtual void Interact_Implementation(AActor* Interactor) override
	{
		UE_LOG(LogTemp, Warning, TEXT("ASBTestInteractableActor::Interact_Implementation called"));
		InteractCount++;
	}

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override
	{
		UE_LOG(LogTemp, Warning, TEXT("ASBTestInteractableActor::GetInteractionPrompt_Implementation called"));
		return CustomPrompt;
	}

	virtual float GetInteractionDuration_Implementation(AActor* Interactor) const override
	{
		UE_LOG(LogTemp, Warning, TEXT("ASBTestInteractableActor::GetInteractionDuration_Implementation called. CustomDuration=%f"), CustomDuration);
		return CustomDuration;
	}

	virtual bool IsInteractionLocked_Implementation(AActor* Interactor) const override
	{
		bool bLocked = CurrentLockOwner.IsValid() && CurrentLockOwner != Interactor;
		UE_LOG(LogTemp, Warning, TEXT("ASBTestInteractableActor::IsInteractionLocked_Implementation called. Locked=%d"), bLocked);
		return bLocked;
	}

	virtual void LockInteraction_Implementation(AActor* Interactor) override
	{
		UE_LOG(LogTemp, Warning, TEXT("ASBTestInteractableActor::LockInteraction_Implementation called"));
		if (!CurrentLockOwner.IsValid())
		{
			CurrentLockOwner = Interactor;
		}
	}

	virtual void UnlockInteraction_Implementation(AActor* Interactor) override
	{
		UE_LOG(LogTemp, Warning, TEXT("ASBTestInteractableActor::UnlockInteraction_Implementation called"));
		if (CurrentLockOwner == Interactor)
		{
			CurrentLockOwner = nullptr;
		}
	}

	virtual void GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const override
	{
		FSBDebugLine Header;
		Header.Label = GetClass()->GetName();
		Header.bIsHeader = true;
		OutDebugLines.Add(Header);

		FSBDebugLine DurationLine;
		DurationLine.Label = TEXT("Interaction Duration");
		DurationLine.Value = FString::SanitizeFloat(CustomDuration);
		OutDebugLines.Add(DurationLine);

		FSBDebugLine LockLine;
		LockLine.Label = TEXT("Is Locked");
		LockLine.Value = CurrentLockOwner.IsValid() ? FString::Printf(TEXT("Yes (Owner: %s)"), *CurrentLockOwner->GetName()) : TEXT("No");
		OutDebugLines.Add(LockLine);

		FSBDebugLine CountLine;
		CountLine.Label = TEXT("Interact Count");
		CountLine.Value = FString::FromInt(InteractCount);
		OutDebugLines.Add(CountLine);
	}
};

UCLASS()
class USBTestInteractionComponent : public USBInteractionComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TObjectPtr<AActor> MockInteractable = nullptr;

	virtual void ScanForInteractables() override
	{
		if (bIsHoldingInteraction)
		{
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("ScanForInteractables: MockInteractable=%s, CurrentInteractableActor=%s"),
			MockInteractable ? *MockInteractable->GetName() : TEXT("nullptr"),
			CurrentInteractableActor ? *CurrentInteractableActor->GetName() : TEXT("nullptr"));

		if (MockInteractable != CurrentInteractableActor)
		{
			if (CurrentInteractableActor)
			{
				if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
				{
					FGameplayTag ClearedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Cleared"));
					EventSubsystem->PublishEvent(ClearedTag, CurrentInteractableActor);
				}
			}

			CurrentInteractableActor = MockInteractable;

			if (CurrentInteractableActor)
			{
				if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
				{
					USBInteractionAvailableEventPayload* Payload = NewObject<USBInteractionAvailableEventPayload>(this);
					Payload->InteractableActor = CurrentInteractableActor;

					Payload->PromptText = Target_GetInteractionPrompt(CurrentInteractableActor);
					Payload->Duration = Target_GetInteractionDuration(CurrentInteractableActor);

					FGameplayTag AvailableTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Available"));
					EventSubsystem->PublishEvent(AvailableTag, Payload);
				}
			}
		}
	}
};

UCLASS()
class USBTestInteractionListener : public UObject
{
	GENERATED_BODY()

public:
	USBTestInteractionListener()
	{
		AvailableCount = 0;
		ClearedCount = 0;
		StartedCount = 0;
		ProgressCount = 0;
		CompletedCount = 0;
		LastProgressVal = 0.0f;
	}

	UPROPERTY()
	int32 AvailableCount;

	UPROPERTY()
	int32 ClearedCount;

	UPROPERTY()
	int32 StartedCount;

	UPROPERTY()
	int32 ProgressCount;

	UPROPERTY()
	int32 CompletedCount;

	UPROPERTY()
	float LastProgressVal;

	UFUNCTION()
	void OnAvailable(FGameplayTag Tag, UObject* Payload)
	{
		AvailableCount++;
	}

	UFUNCTION()
	void OnCleared(FGameplayTag Tag, UObject* Payload)
	{
		ClearedCount++;
	}

	UFUNCTION()
	void OnStarted(FGameplayTag Tag, UObject* Payload)
	{
		StartedCount++;
	}

	UFUNCTION()
	void OnProgress(FGameplayTag Tag, UObject* Payload)
	{
		ProgressCount++;
		if (USBInteractionProgressEventPayload* ProgressPayload = Cast<USBInteractionProgressEventPayload>(Payload))
		{
			LastProgressVal = ProgressPayload->ProgressPercent;
		}
	}

	UFUNCTION()
	void OnCompleted(FGameplayTag Tag, UObject* Payload)
	{
		CompletedCount++;
	}
};
