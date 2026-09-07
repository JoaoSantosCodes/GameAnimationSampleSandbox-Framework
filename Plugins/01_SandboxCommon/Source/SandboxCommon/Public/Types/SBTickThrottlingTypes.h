// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBTickThrottlingTypes.generated.h"

UENUM(BlueprintType)
enum class ESBTickLODLevel : uint8
{
	LOD0_HighPriority    UMETA(DisplayName = "LOD0 High Priority (60Hz)"),
	LOD1_MediumPriority  UMETA(DisplayName = "LOD1 Medium Priority (10Hz)"),
	LOD2_LowPriority     UMETA(DisplayName = "LOD2 Low Priority (2Hz)"),
	LOD3_BackgroundBatch UMETA(DisplayName = "LOD3 Background Batch (0.5Hz)"),
	LOD_Suspended        UMETA(DisplayName = "LOD Suspended")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBTickThrottlingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	float LOD0_MaxDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	float LOD1_MaxDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	float LOD2_MaxDistance = 15000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	float LOD1_Interval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	float LOD2_Interval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	float LOD3_Interval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	bool bEnableCameraFrustumCheck = true;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBTickThrottlingState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	ESBTickLODLevel CurrentLOD = ESBTickLODLevel::LOD0_HighPriority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	float AccumulatedDeltaTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	float TimeSinceLastTick = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	float DistanceToNearestViewer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	bool bShouldTickThisFrame = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throttling")
	int32 TotalTicksExecuted = 0;
};
