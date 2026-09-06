#pragma once

#include "CoreMinimal.h"
#include "SBZiplineTypes.generated.h"

UENUM(BlueprintType)
enum class ESBZiplineState : uint8
{
	None          UMETA(DisplayName = "None"),
	Mounting      UMETA(DisplayName = "Mounting"),
	Sliding       UMETA(DisplayName = "Sliding"),
	Dismounting   UMETA(DisplayName = "Dismounting")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBZiplineRideData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	FVector StartPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	FVector EndPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	float TotalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	float CurrentDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	bool bIsRiding = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBZiplineSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	float BaseSlideSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	float MaxSlideSpeed = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	float GravityAcceleration = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	float DismountLaunchMultiplier = 1.2f;
};
