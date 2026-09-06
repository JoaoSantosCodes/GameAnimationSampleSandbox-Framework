#pragma once

#include "CoreMinimal.h"
#include "SBPipeNetworkTypes.generated.h"

UENUM(BlueprintType)
enum class ESBFluidType : uint8
{
	None         UMETA(DisplayName = "None"),
	Water        UMETA(DisplayName = "Water"),
	CrudeOil     UMETA(DisplayName = "CrudeOil"),
	Fuel         UMETA(DisplayName = "Fuel"),
	Steam        UMETA(DisplayName = "Steam"),
	ToxicGas     UMETA(DisplayName = "ToxicGas")
};

UENUM(BlueprintType)
enum class ESBPipeNodeType : uint8
{
	SourcePump           UMETA(DisplayName = "SourcePump"),
	PipeSegment          UMETA(DisplayName = "PipeSegment"),
	Valve                UMETA(DisplayName = "Valve"),
	FluidTank            UMETA(DisplayName = "FluidTank"),
	GasVent              UMETA(DisplayName = "GasVent"),
	ConsumerApparatus    UMETA(DisplayName = "ConsumerApparatus")
};

UENUM(BlueprintType)
enum class ESBPipeFlowState : uint8
{
	Empty         UMETA(DisplayName = "Empty"),
	Flowing       UMETA(DisplayName = "Flowing"),
	Blocked       UMETA(DisplayName = "Blocked"),
	Pressurized   UMETA(DisplayName = "Pressurized"),
	Leaking       UMETA(DisplayName = "Leaking"),
	Ruptured      UMETA(DisplayName = "Ruptured")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBPipeNodeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	ESBFluidType FluidType = ESBFluidType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	ESBPipeNodeType NodeType = ESBPipeNodeType::PipeSegment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float FluidAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float FluidCapacity = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float CurrentPressure = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float MaxSafePressure = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float FlowRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float ValveOpenPercentage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	bool bIsPumpActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	bool bIsRuptured = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	ESBPipeFlowState FlowState = ESBPipeFlowState::Empty;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBPipeNetworkSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float MaxConnectionDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	int32 MaxPipeConnections = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float PumpPressureGeneration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float BaseFlowSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float RuptureThresholdMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PipeNetwork")
	float LeakLossRate = 50.0f;
};
