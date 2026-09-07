// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBFootIKTypes.generated.h"

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBFootIKTraceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	float FootOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	FRotator FootRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	bool bHit = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBFootIKResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	FSBFootIKTraceData LeftFoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	FSBFootIKTraceData RightFoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	float PelvisOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	bool bIsGrounded = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	bool bIsOnSlope = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBFootIKSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	float TraceDistanceAbove = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	float TraceDistanceBelow = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	float SlopeThresholdDegrees = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	FName LeftFootSocket = FName("foot_l");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
	FName RightFootSocket = FName("foot_r");
};
