#pragma once

#include "CoreMinimal.h"
#include "SBGrappleTypes.generated.h"

UENUM(BlueprintType)
enum class ESBGrappleState : uint8
{
	None        UMETA(DisplayName = "None"),
	Firing      UMETA(DisplayName = "Firing"),
	Pulling     UMETA(DisplayName = "Pulling"),
	Swinging    UMETA(DisplayName = "Swinging"),
	Detaching   UMETA(DisplayName = "Detaching")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBGrappleAnchorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	FVector AnchorLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float InitialCableLength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float CurrentCableLength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	bool bIsAttached = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBGrappleSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float MaxGrappleRange = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float PullSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float SwingAcceleration = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float LaunchImpulseMultiplier = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float MinDetachDistance = 100.0f;
};
