// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBWorldIntegrityTypes.h"
#include "SBWorldIntegrityAuditorComponent.generated.h"

/**
 * Componente auditor para auto-validação de integridade do ator e seus subsistemas
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOXCORE_API USBWorldIntegrityAuditorComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBWorldIntegrityAuditorComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	/** Executa a auditoria de integridade das propriedades do ator */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|WorldIntegrity")
	bool RunAudit();

	/** Força o registro de um erro de integridade de teste */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|WorldIntegrity")
	void InjectTestIssue(FName IssueId, const FString& Description, ESBIntegritySeverity Severity, bool bAutoFixable);

	/** Retorna se o componente foi auditado e está limpo */
	UFUNCTION(BlueprintPure, Category = "Sandbox|WorldIntegrity")
	bool IsClean() const { return bAuditPassed; }

private:
	bool bAuditPassed = true;
	void SyncTags();
};
