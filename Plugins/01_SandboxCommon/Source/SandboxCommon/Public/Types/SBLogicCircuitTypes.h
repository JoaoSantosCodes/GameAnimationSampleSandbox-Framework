// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBLogicCircuitTypes.generated.h"

UENUM(BlueprintType)
enum class ESBLogicNodeType : uint8
{
	LogicGateAND         UMETA(DisplayName = "LogicGateAND"),
	LogicGateOR          UMETA(DisplayName = "LogicGateOR"),
	LogicGateNOT         UMETA(DisplayName = "LogicGateNOT"),
	LogicGateXOR         UMETA(DisplayName = "LogicGateXOR"),
	LogicGateNAND        UMETA(DisplayName = "LogicGateNAND"),
	LogicGateNOR         UMETA(DisplayName = "LogicGateNOR"),
	Comparator           UMETA(DisplayName = "Comparator"),
	ArithmeticProcessor  UMETA(DisplayName = "ArithmeticProcessor"),
	Counter              UMETA(DisplayName = "Counter"),
	PulseGenerator       UMETA(DisplayName = "PulseGenerator"),
	RSLatch              UMETA(DisplayName = "RSLatch"),
	SensorInput          UMETA(DisplayName = "SensorInput"),
	ActuatorOutput       UMETA(DisplayName = "ActuatorOutput")
};

UENUM(BlueprintType)
enum class ESBLogicComparisonOp : uint8
{
	GreaterThan          UMETA(DisplayName = "GreaterThan (>)"),
	LessThan             UMETA(DisplayName = "LessThan (<)"),
	Equal                UMETA(DisplayName = "Equal (==)"),
	NotEqual             UMETA(DisplayName = "NotEqual (!=)"),
	GreaterOrEqual       UMETA(DisplayName = "GreaterOrEqual (>=)"),
	LessOrEqual          UMETA(DisplayName = "LessOrEqual (<=)")
};

UENUM(BlueprintType)
enum class ESBLogicArithmeticOp : uint8
{
	Add                  UMETA(DisplayName = "Add (+)"),
	Subtract             UMETA(DisplayName = "Subtract (-)"),
	Multiply             UMETA(DisplayName = "Multiply (*)"),
	Divide               UMETA(DisplayName = "Divide (/)"),
	Modulo               UMETA(DisplayName = "Modulo (%)")
};

UENUM(BlueprintType)
enum class ESBLogicWireColor : uint8
{
	RedWire              UMETA(DisplayName = "RedWire"),
	GreenWire            UMETA(DisplayName = "GreenWire"),
	CopperWire           UMETA(DisplayName = "CopperWire")
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBCircuitSignal
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	FName SignalChannel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	float SignalValue = 0.0f;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLogicGateData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	ESBLogicNodeType NodeType = ESBLogicNodeType::Comparator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	ESBLogicComparisonOp ComparisonOp = ESBLogicComparisonOp::GreaterThan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	ESBLogicArithmeticOp ArithmeticOp = ESBLogicArithmeticOp::Add;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	FName InputChannelA = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	FName InputChannelB = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	float ConstantOperand = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	bool bUseConstantOperand = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	FName OutputChannel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	float OutputValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	bool bConditionEvaluatedTrue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	bool bLatchState = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	int32 CounterCurrent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	int32 CounterTarget = 10;
};

USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLogicCircuitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	float EvaluationFrequency = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logic")
	float PulseDuration = 0.2f;
};
