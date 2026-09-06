#pragma once

#include "CoreMinimal.h"
#include "SBGliderTypes.generated.h"

UENUM(BlueprintType)
enum class ESBGliderState : uint8
{
	Retracted   UMETA(DisplayName = "Retracted"),
	Deploying   UMETA(DisplayName = "Deploying"),
	Gliding     UMETA(DisplayName = "Gliding"),
	Diving      UMETA(DisplayName = "Diving")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBGliderFlightData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	float CurrentFallSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	float CurrentForwardSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	float CurrentPitchAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	float StaminaCostPerSec = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	bool bIsGliding = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBGliderSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	float MinDeployHeight = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	float GlideFallSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	float GlideForwardSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	float DiveFallSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	float DiveForwardSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glider")
	float StaminaDrainPerSec = 8.0f;
};
