#pragma once

#include "CoreMinimal.h"
#include "SBSwimTypes.generated.h"

UENUM(BlueprintType)
enum class ESBSwimState : uint8
{
	None              UMETA(DisplayName = "None"),
	SurfaceSwimming   UMETA(DisplayName = "Surface Swimming"),
	Diving            UMETA(DisplayName = "Diving")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBOxygenData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen")
	float CurrentOxygen = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen")
	float MaxOxygen = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen")
	float DepletionRatePerSec = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen")
	float RecoveryRatePerSec = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen")
	float DrowningDamagePerSec = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oxygen")
	bool bIsDrowning = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBSwimSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
	float SurfaceSwimSpeed = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
	float DiveSpeed = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
	float SprintSwimSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
	float WaterSurfaceTolerance = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
	float BuoyancyForce = 980.0f;
};
