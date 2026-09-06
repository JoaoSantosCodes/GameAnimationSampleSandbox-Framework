#pragma once

#include "CoreMinimal.h"
#include "SBMachineryTypes.generated.h"

UENUM(BlueprintType)
enum class ESBMachineryType : uint8
{
	Excavator     UMETA(DisplayName = "Excavator"),
	Crane         UMETA(DisplayName = "Crane"),
	Bulldozer     UMETA(DisplayName = "Bulldozer"),
	Loader        UMETA(DisplayName = "Loader")
};

UENUM(BlueprintType)
enum class ESBMachineryState : uint8
{
	Parked        UMETA(DisplayName = "Parked"),
	Idling        UMETA(DisplayName = "Idling"),
	Operating     UMETA(DisplayName = "Operating"),
	Lifting       UMETA(DisplayName = "Lifting"),
	Excavating    UMETA(DisplayName = "Excavating")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMachineryHydraulicData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float SystemPressure = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float MaxSystemPressure = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float PumpRPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float BoomAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float ArmAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float BucketAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float CabinSlewAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float CableLength = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float LiftedPayloadMass = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	bool bEngineRunning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	bool bOutriggersDeployed = false;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBMachinerySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float MaxLiftCapacity = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float BoomSlewSpeed = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float HydraulicBuildRate = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float HydraulicReliefPressure = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float WinchSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Machinery")
	float ExcavationForce = 25000.0f;
};
