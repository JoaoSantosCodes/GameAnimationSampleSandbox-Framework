// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBAircraftTypes.generated.h"

UENUM(BlueprintType)
enum class ESBAircraftType : uint8
{
	FixedWing     UMETA(DisplayName = "FixedWing"),
	Helicopter    UMETA(DisplayName = "Helicopter"),
	VTOL          UMETA(DisplayName = "VTOL")
};

UENUM(BlueprintType)
enum class ESBFlightState : uint8
{
	Parked        UMETA(DisplayName = "Parked"),
	Taxiing       UMETA(DisplayName = "Taxiing"),
	Takeoff       UMETA(DisplayName = "Takeoff"),
	Airborne      UMETA(DisplayName = "Airborne"),
	Stalling      UMETA(DisplayName = "Stalling"),
	Landing       UMETA(DisplayName = "Landing")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAircraftFlightData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float Airspeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float Altitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float ThrottleInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float PitchInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float RollInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float YawInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float LiftCoefficient = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	bool bIsAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	bool bIsStalling = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	bool bVTOLMode = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBAircraftSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float MaxThrustSpeed = 3500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float AccelerationRate = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float StallSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float PitchRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float RollRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float YawRate = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft")
	float DragCoefficient = 0.05f;
};
