// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBGameplayEffectTypes.generated.h"

UENUM(BlueprintType)
enum class ESBEffectDurationType : uint8
{
	Instant UMETA(DisplayName = "Instant"),
	Infinite UMETA(DisplayName = "Infinite"),
	HasDuration UMETA(DisplayName = "Has Duration")
};

UENUM(BlueprintType)
enum class ESBEffectModifierOp : uint8
{
	Add UMETA(DisplayName = "Add"),
	Multiply UMETA(DisplayName = "Multiply"),
	Override UMETA(DisplayName = "Override")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBGameplayEffectModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	FGameplayTag AttributeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	ESBEffectModifierOp ModifierOp = ESBEffectModifierOp::Add;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	float Magnitude = 0.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBGameplayEffectSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	FGameplayTag EffectTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	FText EffectDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	ESBEffectDurationType DurationType = ESBEffectDurationType::HasDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	float Duration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	float Period = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	int32 MaxStacks = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	TArray<FSBGameplayEffectModifier> Modifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	FGameplayTagContainer GrantedTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	FGameplayTagContainer ImmunityTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
	FGameplayTagContainer RemoveEffectsWithTags;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBActiveGameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameplayEffect")
	FSBGameplayEffectSpec Spec;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameplayEffect")
	float TimeRemaining = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameplayEffect")
	float PeriodTimer = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameplayEffect")
	int32 CurrentStacks = 1;
};
