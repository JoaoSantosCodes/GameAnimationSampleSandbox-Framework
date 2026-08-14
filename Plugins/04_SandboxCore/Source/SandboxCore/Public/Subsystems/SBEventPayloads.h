#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "SBEventPayloads.generated.h"

UCLASS(BlueprintType)
class SANDBOXCORE_API USBPawnEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<APawn> TargetPawn = nullptr;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBAttributeChangedPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	FGameplayTag AttributeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	float BaseValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	float CurrentValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	float MaxValue = 0.0f;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBInteractionAvailableEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<AActor> InteractableActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	float Duration = 0.0f;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBInteractionProgressEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<AActor> InteractableActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	float ProgressPercent = 0.0f;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBInventoryEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<UObject> ItemInstance = nullptr;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBCooldownEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	float Duration = 0.0f;
};
