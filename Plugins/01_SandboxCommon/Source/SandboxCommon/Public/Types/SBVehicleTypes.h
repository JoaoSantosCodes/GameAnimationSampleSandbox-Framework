// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBVehicleTypes.generated.h"

UENUM(BlueprintType)
enum class ESBVehicleType : uint8
{
	Wheeled       UMETA(DisplayName = "Wheeled"),
	Hover         UMETA(DisplayName = "Hover"),
	Tracked       UMETA(DisplayName = "Tracked")
};

UENUM(BlueprintType)
enum class ESBVehicleSeat : uint8
{
	Driver              UMETA(DisplayName = "Driver"),
	PassengerFront      UMETA(DisplayName = "Passenger Front"),
	PassengerRearLeft   UMETA(DisplayName = "Passenger Rear Left"),
	PassengerRearRight  UMETA(DisplayName = "Passenger Rear Right")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBVehicleSeatOccupant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	TWeakObjectPtr<AActor> OccupantActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	ESBVehicleSeat Seat = ESBVehicleSeat::Driver;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBVehicleDrivetrainData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float ThrottleInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float SteeringInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	bool bHandbrakeActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float CurrentFuel = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float MaxFuel = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	bool bEngineRunning = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBVehicleSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float MaxForwardSpeed = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float MaxReverseSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float AccelerationRate = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float BrakingDeceleration = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float NaturalFriction = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float SteeringSensitivity = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float FuelConsumptionRate = 1.5f;
};
