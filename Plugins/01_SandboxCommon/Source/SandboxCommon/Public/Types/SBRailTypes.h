// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Types/SBPipeNetworkTypes.h"
#include "SBRailTypes.generated.h"

UENUM(BlueprintType)
enum class ESBRailNodeType : uint8
{
	StraightTrack    UMETA(DisplayName = "StraightTrack"),
	CurvedTrack      UMETA(DisplayName = "CurvedTrack"),
	SwitchBranch     UMETA(DisplayName = "SwitchBranch"),
	BlockSignal      UMETA(DisplayName = "BlockSignal"),
	ChainSignal      UMETA(DisplayName = "ChainSignal"),
	TrainStation     UMETA(DisplayName = "TrainStation")
};

UENUM(BlueprintType)
enum class ESBRailSignalState : uint8
{
	ClearGreen       UMETA(DisplayName = "ClearGreen"),
	ApproachYellow   UMETA(DisplayName = "ApproachYellow"),
	StopRed          UMETA(DisplayName = "StopRed")
};

UENUM(BlueprintType)
enum class ESBTrainMovementState : uint8
{
	Stationary       UMETA(DisplayName = "Stationary"),
	Traveling        UMETA(DisplayName = "Traveling"),
	Loading          UMETA(DisplayName = "Loading"),
	Unloading        UMETA(DisplayName = "Unloading"),
	WaitingSignal    UMETA(DisplayName = "WaitingSignal"),
	Derailed         UMETA(DisplayName = "Derailed")
};

UENUM(BlueprintType)
enum class ESBWagonType : uint8
{
	LocomotiveEngine UMETA(DisplayName = "LocomotiveEngine"),
	FreightCargo     UMETA(DisplayName = "FreightCargo"),
	FluidTanker      UMETA(DisplayName = "FluidTanker")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBRailWagonData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	FName WagonId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	ESBWagonType WagonType = ESBWagonType::LocomotiveEngine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	TMap<FName, int32> CargoInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	int32 CargoCapacity = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	ESBFluidType FluidType = ESBFluidType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float FluidAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float FluidCapacity = 1000.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBTrainConsist
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	FName TrainId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	ESBTrainMovementState MovementState = ESBTrainMovementState::Stationary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float MaxSpeed = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float CurrentTrackProgressAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	int32 CurrentTrackSegmentId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	TArray<FSBRailWagonData> Wagons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	TArray<FName> DestinationStations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	int32 CurrentStationIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float StationWaitTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float MaxStationWaitDuration = 3.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBRailBlockData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	int32 BlockId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	bool bIsOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	FName OccupyingTrainId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	ESBRailSignalState SignalState = ESBRailSignalState::ClearGreen;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBRailNetworkSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float Acceleration = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float Deceleration = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float StationTransferRate = 50.0f;
};
