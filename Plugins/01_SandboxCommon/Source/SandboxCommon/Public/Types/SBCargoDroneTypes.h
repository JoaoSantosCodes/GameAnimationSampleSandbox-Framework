// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBCargoDroneTypes.generated.h"

UENUM(BlueprintType)
enum class ESBCargoDroneFlightState : uint8
{
	IdleAtPort           UMETA(DisplayName = "IdleAtPort"),
	TakingOff            UMETA(DisplayName = "TakingOff"),
	InFlight             UMETA(DisplayName = "InFlight"),
	Landing              UMETA(DisplayName = "Landing"),
	Recharging           UMETA(DisplayName = "Recharging"),
	LowBatteryReturn     UMETA(DisplayName = "LowBatteryReturn")
};

UENUM(BlueprintType)
enum class ESBCargoDroneModel : uint8
{
	LightCourier         UMETA(DisplayName = "LightCourier"),
	HeavyLiftDrone       UMETA(DisplayName = "HeavyLiftDrone"),
	LongRangeHexacopter  UMETA(DisplayName = "LongRangeHexacopter")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCargoDroneData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	FName DroneId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	ESBCargoDroneModel DroneModel = ESBCargoDroneModel::LightCourier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	ESBCargoDroneFlightState FlightState = ESBCargoDroneFlightState::IdleAtPort;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	FVector CurrentLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	FVector HomePortLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	FVector TargetPortLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	FName HomePortId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	FName TargetPortId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float CurrentBattery = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float MaxBattery = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float BatteryDischargeRate = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float BatteryRechargeRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float LowBatteryThreshold = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float FlightSpeed = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float CruiseAltitude = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float FlightProgressAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	TMap<FName, int32> CargoBay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	int32 CargoBayCapacity = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	int32 TotalTripsCompleted = 0;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBDronePortData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	FName PortId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	FVector PortLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	bool bHasRechargeDock = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	TMap<FName, int32> InputBuffer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	TMap<FName, int32> OutputBuffer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	int32 BufferCapacity = 200;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCargoDroneSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float TakeoffDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float LandingDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone")
	float TransferRate = 20.0f;
};
