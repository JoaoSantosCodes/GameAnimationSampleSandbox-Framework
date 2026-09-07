// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBStateMatrixTypes.h"
#include "SBStateMatrixGuardComponent.generated.h"

/**
 * Componente guardião que assegura consistência da composição de tags em tempo de execução
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOXCORE_API USBStateMatrixGuardComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBStateMatrixGuardComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	/** Se verdadeiro, aplica verificações estritas da matriz em todas as tentativas de transição */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|StateMatrix")
	bool bStrictEnforcement = true;

	/** Tenta aplicar uma tag no StateComponent validando contra conflitos da matriz */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|StateMatrix")
	bool TryApplyStateTag(FGameplayTag TagToApply);

	/** Retorna o total de transições com conflito interceptadas localmente */
	UFUNCTION(BlueprintPure, Category = "Sandbox|StateMatrix")
	int32 GetInterceptedConflictCount() const { return InterceptedConflictCount; }

private:
	int32 InterceptedConflictCount = 0;
	void SyncTags();
};
