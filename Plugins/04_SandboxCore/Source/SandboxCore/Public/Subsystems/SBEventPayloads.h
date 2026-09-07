// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Chaos/ChaosEngineInterface.h"
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

UCLASS(BlueprintType)
class SANDBOXCORE_API USBHitReactEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	FName HitBoneName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	FVector HitDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	float DamageDealt = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	bool bIsCritical = false;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBCraftingEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	FGameplayTag RecipeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<UObject> ResultItemDef = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	int32 ResultQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	bool bSuccess = false;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBFootstepEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TEnumAsByte<EPhysicalSurface> SurfaceType = SurfaceType_Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	FName FootSocketName = NAME_None;
};

UCLASS(BlueprintType)
class SANDBOXCORE_API USBQuestRewardsPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<APawn> TargetPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
	TObjectPtr<UObject> QuestData = nullptr;
};
