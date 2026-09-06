#pragma once

#include "CoreMinimal.h"
#include "SBSpacecraftTypes.generated.h"

UENUM(BlueprintType)
enum class ESBSpacecraftType : uint8
{
	Fighter       UMETA(DisplayName = "Fighter"),
	Freighter     UMETA(DisplayName = "Freighter"),
	Shuttle       UMETA(DisplayName = "Shuttle")
};

UENUM(BlueprintType)
enum class ESBSpaceflightState : uint8
{
	Docked        UMETA(DisplayName = "Docked"),
	Cruising      UMETA(DisplayName = "Cruising"),
	Boosting      UMETA(DisplayName = "Boosting"),
	Drifting      UMETA(DisplayName = "Drifting"),
	Reentry       UMETA(DisplayName = "Reentry")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSpacecraftFlightData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	FVector LinearVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	FVector AngularVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	FVector TranslationInput = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	FVector RotationInput = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	float BoostMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	float HeatShieldIntegrity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	bool bFlightAssistActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	bool bBoostActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	bool bInReentry = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSpacecraftSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	float MaxLinearSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	float BoostMaxSpeed = 12000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	float LinearAcceleration = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	float AngularAcceleration = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	float InertiaDampingRate = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft")
	float HeatDissipationRate = 10.0f;
};
