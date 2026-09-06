#pragma once

#include "CoreMinimal.h"
#include "SBMotionWarpTypes.generated.h"

UENUM(BlueprintType)
enum class ESBMotionWarpState : uint8
{
	Inactive UMETA(DisplayName = "Inactive"),
	Warping UMETA(DisplayName = "Warping"),
	Completed UMETA(DisplayName = "Completed"),
	Aborted UMETA(DisplayName = "Aborted")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMotionWarpTarget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionWarp")
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionWarp")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionWarp")
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionWarp")
	float TargetOffsetDistance = 120.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMotionWarpConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionWarp")
	float MaxWarpDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionWarp")
	float MinWarpDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionWarp")
	float WarpDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionWarp")
	bool bWarpTranslation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionWarp")
	bool bWarpRotation = true;
};
