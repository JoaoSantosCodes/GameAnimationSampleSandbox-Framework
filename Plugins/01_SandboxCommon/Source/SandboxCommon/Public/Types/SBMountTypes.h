// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBMountTypes.generated.h"

UENUM(BlueprintType)
enum class ESBMountState : uint8
{
	Unmounted     UMETA(DisplayName = "Unmounted"),
	Mounting      UMETA(DisplayName = "Mounting"),
	Mounted       UMETA(DisplayName = "Mounted"),
	Dismounting   UMETA(DisplayName = "Dismounting")
};

UENUM(BlueprintType)
enum class ESBMountGait : uint8
{
	Walk     UMETA(DisplayName = "Walk"),
	Trot     UMETA(DisplayName = "Trot"),
	Canter   UMETA(DisplayName = "Canter"),
	Gallop   UMETA(DisplayName = "Gallop")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMountRiderData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	TWeakObjectPtr<AActor> RiderActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	TWeakObjectPtr<AActor> MountActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	FName SaddleSocketName = FName("saddle_socket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	ESBMountState MountState = ESBMountState::Unmounted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	ESBMountGait CurrentGait = ESBMountGait::Walk;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMountSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	float MountInteractDistance = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	float GallopStaminaCostPerSecond = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	float TrotSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	float GallopSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	FName DefaultSaddleSocket = FName("saddle_socket");
};
