// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SBFaultToleranceTypes.generated.h"

/**
 * Modo operacional do serviço ou subsistema em contingência
 */
UENUM(BlueprintType)
enum class ESBFaultToleranceMode : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Degraded UMETA(DisplayName = "Degraded"),
	Fallback UMETA(DisplayName = "Fallback"),
	Disabled UMETA(DisplayName = "Disabled")
};

/**
 * Registro de monitoramento e contingência de um serviço
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBFallbackServiceRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	FName ServiceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	FGameplayTag ServiceTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	ESBFaultToleranceMode CurrentMode = ESBFaultToleranceMode::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	int32 FailureCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	float FallbackDefaultValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	bool bIsHealthy = true;

	FSBFallbackServiceRecord() = default;

	FSBFallbackServiceRecord(FName InName, FGameplayTag InTag, float InFallbackVal = 0.0f)
		: ServiceName(InName)
		, ServiceTag(InTag)
		, CurrentMode(ESBFaultToleranceMode::Normal)
		, FailureCount(0)
		, FallbackDefaultValue(InFallbackVal)
		, bIsHealthy(true)
	{
	}
};

/**
 * Métricas do subsistema de tolerância a falhas
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBFaultToleranceMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	int32 TotalRegisteredServices = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	int32 ActiveDegradedServices = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	int32 ActiveFallbackServices = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	int32 TotalFaultsIntercepted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaultTolerance")
	int32 TotalRecoveries = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBOnServiceStateChanged, FName, ServiceName, ESBFaultToleranceMode, NewMode, bool, bIsHealthy);
