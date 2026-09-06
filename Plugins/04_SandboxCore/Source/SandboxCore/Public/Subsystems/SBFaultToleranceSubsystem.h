#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBFaultToleranceTypes.h"
#include "SBFaultToleranceSubsystem.generated.h"

/**
 * Subsistema centralizado de resiliência, fallback dinâmico e tolerância a falhas
 */
UCLASS()
class SANDBOXCORE_API USBFaultToleranceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBFaultToleranceSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Registra um serviço com valor de contingência padrão */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|FaultTolerance")
	void RegisterService(FName ServiceName, FGameplayTag ServiceTag, float DefaultFallbackValue = 0.0f);

	/** Reporta falha em um serviço e ajusta o modo operacional se ultrapassar o limiar */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|FaultTolerance")
	void ReportServiceFailure(FName ServiceName);

	/** Reporta recuperação de um serviço, restaurando o modo Normal */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|FaultTolerance")
	void ReportServiceRecovered(FName ServiceName);

	/** Define explicitamente o modo operacional de um serviço */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|FaultTolerance")
	void SetServiceMode(FName ServiceName, ESBFaultToleranceMode NewMode);

	/** Retorna o modo operacional atual de um serviço */
	UFUNCTION(BlueprintPure, Category = "Sandbox|FaultTolerance")
	ESBFaultToleranceMode GetServiceMode(FName ServiceName) const;

	/** Retorna se o serviço está saudável */
	UFUNCTION(BlueprintPure, Category = "Sandbox|FaultTolerance")
	bool IsServiceHealthy(FName ServiceName) const;

	/** Consulta um valor com proteção de fallback contra nulos ou instabilidade */
	UFUNCTION(BlueprintPure, Category = "Sandbox|FaultTolerance")
	float QuerySafeValue(FName ServiceName, float RawValue, bool bServiceValid = true) const;

	/** Retorna as métricas operacionais */
	UFUNCTION(BlueprintPure, Category = "Sandbox|FaultTolerance")
	FSBFaultToleranceMetrics GetMetrics() const;

	/** Reinicializa todos os serviços e métricas */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|FaultTolerance")
	void ResetSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|FaultTolerance")
	FSBOnServiceStateChanged OnServiceStateChanged;

private:
	TMap<FName, FSBFallbackServiceRecord> MonitoredServices;

	int32 TotalFaultsIntercepted = 0;
	int32 TotalRecoveries = 0;
	const int32 DegradedThreshold = 2;
	const int32 FallbackThreshold = 4;
};
