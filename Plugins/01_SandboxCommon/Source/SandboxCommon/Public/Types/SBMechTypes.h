// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBMechTypes.generated.h"

UENUM(BlueprintType)
enum class ESBMechClass : uint8
{
	LightScout     UMETA(DisplayName = "Light Scout"),
	MediumAssault  UMETA(DisplayName = "Medium Assault"),
	HeavySiege     UMETA(DisplayName = "Heavy Siege")
};

UENUM(BlueprintType)
enum class ESBMechState : uint8
{
	PoweredOff     UMETA(DisplayName = "Powered Off"),
	Idle           UMETA(DisplayName = "Idle"),
	Walking        UMETA(DisplayName = "Walking"),
	JumpJets       UMETA(DisplayName = "Jump Jets"),
	Dashing        UMETA(DisplayName = "Dashing"),
	Overheated     UMETA(DisplayName = "Overheated"),
	Ejected        UMETA(DisplayName = "Ejected")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMechOperationalData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float CoreHeat = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float MaxCoreHeat = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float JumpJetFuel = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float MaxJumpJetFuel = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	FVector MoveInput = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	bool bIsPowered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	bool bJumpJetsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	bool bIsOverheated = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMechSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float MaxWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float DashSpeed = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float JumpJetVerticalThrust = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float HeatGenerationRate = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float HeatCoolingRate = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float OverheatThreshold = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float OverheatRecoveryThreshold = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float JumpJetFuelDrainRate = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech")
	float JumpJetFuelRechargeRate = 20.0f;
};
