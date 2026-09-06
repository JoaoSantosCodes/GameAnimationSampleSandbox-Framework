#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBFaultToleranceTypes.h"
#include "SBFaultResilientComponent.generated.h"

/**
 * Componente que garante resiliência a falhas e contingência de dados para atores
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOXCORE_API USBFaultResilientComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBFaultResilientComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	/** Nome do serviço associado a este componente */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|FaultTolerance")
	FName MonitoredService = TEXT("WeatherService");

	/** Valor seguro padrão caso o serviço falhe */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|FaultTolerance")
	float SafeDefaultValue = 20.0f;

	/** Avalia um valor recebido com garantia de fallback sem causar crashes */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|FaultTolerance")
	float SafeEvaluate(float IncomingValue, bool bPrimaryAvailable = true);

	/** Retorna o último valor avaliado */
	UFUNCTION(BlueprintPure, Category = "Sandbox|FaultTolerance")
	float GetLastEvaluatedValue() const { return LastEvaluatedValue; }

	/** Retorna se o componente está operando em fallback */
	UFUNCTION(BlueprintPure, Category = "Sandbox|FaultTolerance")
	bool IsInFallbackMode() const { return bInFallbackMode; }

private:
	float LastEvaluatedValue = 0.0f;
	bool bInFallbackMode = false;

	void SyncTags();
};
