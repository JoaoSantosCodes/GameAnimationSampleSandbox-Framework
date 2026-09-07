// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBSpaceElevatorTypes.generated.h"

UENUM(BlueprintType)
enum class ESBSpaceElevatorState : uint8
{
	Idle                     UMETA(DisplayName = "Idle"),
	LoadingPayload           UMETA(DisplayName = "LoadingPayload"),
	Ascending                UMETA(DisplayName = "Ascending"),
	DockedAtOrbitalStation   UMETA(DisplayName = "DockedAtOrbitalStation"),
	Descending               UMETA(DisplayName = "Descending"),
	Delivering               UMETA(DisplayName = "Delivering")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSpaceElevatorPhaseRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	int32 PhaseIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	FName RequirementName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	TMap<FName, int32> RequiredItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	TMap<FName, int32> DepositedItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	bool bIsPhaseCompleted = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSpaceElevatorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	int32 CurrentPhaseIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	int32 MaxPhaseCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	ESBSpaceElevatorState State = ESBSpaceElevatorState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	float PodAltitudeAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	float PodAscentSpeed = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	float PodDescentSpeed = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	float PowerDemandMW = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	bool bHasPowerSupply = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	TArray<FSBSpaceElevatorPhaseRequirement> Phases;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSpaceElevatorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	float OrbitalStationWaitTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceElevator")
	float LaunchCountdownDuration = 1.0f;
};
