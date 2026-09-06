#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBWorldIntegrityTypes.h"
#include "SBWorldIntegritySubsystem.generated.h"

/**
 * Subsistema central de validação e auditoria de integridade do mundo procedural e estático
 */
UCLASS()
class SANDBOXCORE_API USBWorldIntegritySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBWorldIntegritySubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Registra um problema de integridade encontrado */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|WorldIntegrity")
	void RegisterIssue(const FSBIntegrityIssue& Issue);

	/** Valida se uma receita possui insumos e produtos válidos */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|WorldIntegrity")
	bool AuditRecipe(FName RecipeId, int32 IngredientCount, int32 OutputCount);

	/** Valida se uma tabela de loot possui pesos positivos */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|WorldIntegrity")
	bool AuditLootTable(FName LootTableId, float TotalWeight);

	/** Valida conexões de malha elétrica ou fluidos */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|WorldIntegrity")
	bool AuditNetworkConnection(FName NodeId, bool bHasPowerSource, bool bHasConsumer);

	/** Executa reparo automático em issues suportados */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|WorldIntegrity")
	int32 AutoFixIssues();

	/** Gera o relatório consolidado de auditoria */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|WorldIntegrity")
	FSBWorldIntegrityReport GenerateReport();

	/** Retorna o último relatório gerado */
	UFUNCTION(BlueprintPure, Category = "Sandbox|WorldIntegrity")
	FSBWorldIntegrityReport GetLastReport() const { return LastReport; }

	/** Reinicializa o subsistema */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|WorldIntegrity")
	void ResetSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|WorldIntegrity")
	FSBOnIntegrityReportGenerated OnIntegrityReportGenerated;

private:
	TArray<FSBIntegrityIssue> RegisteredIssues;
	FSBWorldIntegrityReport LastReport;
};
