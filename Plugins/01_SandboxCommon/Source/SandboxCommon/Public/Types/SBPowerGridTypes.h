#pragma once

#include "CoreMinimal.h"
#include "SBPowerGridTypes.generated.h"

UENUM(BlueprintType)
enum class ESBPowerNodeType : uint8
{
	Generator     UMETA(DisplayName = "Generator"),
	Battery       UMETA(DisplayName = "Battery"),
	Consumer      UMETA(DisplayName = "Consumer"),
	RelayPole     UMETA(DisplayName = "RelayPole")
};

UENUM(BlueprintType)
enum class ESBPowerGridState : uint8
{
	Unpowered     UMETA(DisplayName = "Unpowered"),
	Powered       UMETA(DisplayName = "Powered"),
	Charging      UMETA(DisplayName = "Charging"),
	Discharging   UMETA(DisplayName = "Discharging"),
	Overloaded    UMETA(DisplayName = "Overloaded")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBPowerNodeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	ESBPowerNodeType NodeType = ESBPowerNodeType::Consumer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float PowerGeneration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float PowerConsumption = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float BatteryStoredEnergy = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float BatteryCapacity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float GridTotalProduction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float GridTotalDemand = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float PowerSatisfactionRatio = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	bool bIsBreakerTripped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	ESBPowerGridState GridState = ESBPowerGridState::Unpowered;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBPowerGridSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float MaxConnectionDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	int32 MaxWireConnections = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float OverloadThreshold = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float BatteryChargeRate = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerGrid")
	float BatteryDischargeRate = 1000.0f;
};
