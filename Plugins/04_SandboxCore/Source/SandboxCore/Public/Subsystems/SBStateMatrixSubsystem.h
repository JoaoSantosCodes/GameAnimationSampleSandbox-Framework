// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBStateMatrixTypes.h"
#include "SBStateMatrixSubsystem.generated.h"

/**
 * Subsistema central de verificação e aplicação da matriz de exclusão mútua de estados
 */
UCLASS()
class SANDBOXCORE_API USBStateMatrixSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBStateMatrixSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Registra uma regra de exclusão mútua na matriz */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|StateMatrix")
	void RegisterExclusionRule(const FSBTagMutualExclusionRule& Rule);

	/** Registra conjunto padrão de regras da arquitetura Sandbox */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|StateMatrix")
	void RegisterStandardRules();

	/** Valida a adição de uma nova tag contra o conjunto atual de tags do ator */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|StateMatrix")
	bool ValidateTagAddition(AActor* TargetActor, const FGameplayTag& NewTag, const FGameplayTagContainer& CurrentTags, FGameplayTagContainer& OutPrunedTags);

	/** Retorna as métricas operacionais */
	UFUNCTION(BlueprintPure, Category = "Sandbox|StateMatrix")
	FSBStateMatrixMetrics GetMetrics() const;

	/** Reinicializa as regras e métricas */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|StateMatrix")
	void ResetSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|StateMatrix")
	FSBOnMatrixViolationDetected OnMatrixViolationDetected;

private:
	TArray<FSBTagMutualExclusionRule> RegisteredRules;

	int32 TotalEvaluations = 0;
	int32 TotalViolationsDetected = 0;
	int32 TotalTransitionsBlocked = 0;
	int32 TotalTagsAutoPruned = 0;
};
