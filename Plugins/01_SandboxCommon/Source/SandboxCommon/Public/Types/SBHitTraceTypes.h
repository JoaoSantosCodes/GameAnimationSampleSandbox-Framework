// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "SBHitTraceTypes.generated.h"

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBHitTraceSocketConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	float TraceRadius = 12.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBHitTraceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	TArray<FSBHitTraceSocketConfig> Sockets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	bool bUseSphereSweep = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	float BaseDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	FGameplayTag AttackTag;
};
