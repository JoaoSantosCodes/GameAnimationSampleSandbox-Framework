#pragma once

#include "CoreMinimal.h"
#include "SBWatercraftTypes.generated.h"

UENUM(BlueprintType)
enum class ESBWatercraftType : uint8
{
	Motorboat     UMETA(DisplayName = "Motorboat"),
	Sailboat      UMETA(DisplayName = "Sailboat"),
	Rowboat       UMETA(DisplayName = "Rowboat")
};

UENUM(BlueprintType)
enum class ESBWatercraftState : uint8
{
	Docked        UMETA(DisplayName = "Docked"),
	Cruising      UMETA(DisplayName = "Cruising"),
	Anchored      UMETA(DisplayName = "Anchored"),
	Drifting      UMETA(DisplayName = "Drifting")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBWatercraftNavigationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float ThrottleInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float RudderInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float WaterLevelZ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float BuoyancyDepth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	bool bIsAnchored = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	bool bInWater = true;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBWatercraftSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float MaxForwardSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float MaxReverseSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float AccelerationRate = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float WaterResistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float RudderTurnRate = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watercraft")
	float BuoyancyStiffness = 500.0f;
};
