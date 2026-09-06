#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "SBLockOnTypes.generated.h"

UENUM(BlueprintType)
enum class ESBLockOnSwitchDirection : uint8
{
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLockOnSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float LockDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float BreakDistance = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float MaxAngleDegrees = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float RotationInterpSpeed = 10.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLockOnCandidate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LockOn")
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "LockOn")
	float Distance = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "LockOn")
	float AngleDegrees = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "LockOn")
	float SignedHorizontalAngle = 0.0f;
};
