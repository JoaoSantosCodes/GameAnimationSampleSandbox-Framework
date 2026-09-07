// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "SBDismembermentTypes.generated.h"

UENUM(BlueprintType)
enum class ESBLimbType : uint8
{
	Head UMETA(DisplayName = "Head"),
	LeftArm UMETA(DisplayName = "LeftArm"),
	RightArm UMETA(DisplayName = "RightArm"),
	LeftLeg UMETA(DisplayName = "LeftLeg"),
	RightLeg UMETA(DisplayName = "RightLeg"),
	Torso UMETA(DisplayName = "Torso")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLimbDismemberDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	ESBLimbType LimbType = ESBLimbType::Head;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	FName BoneName = FName("head");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	FName SocketName = FName("neck_socket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	TObjectPtr<UStaticMesh> CapMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	TObjectPtr<UStaticMesh> SeveredLimbMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	bool bIsSevered = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSeverLimbRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	ESBLimbType LimbType = ESBLimbType::Head;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	FVector ImpulseDirection = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	float ImpulseStrength = 500.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBDismembermentSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	TArray<FSBLimbDismemberDefinition> Limbs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismemberment")
	bool bAllowFatalDismembermentOnly = true;
};
