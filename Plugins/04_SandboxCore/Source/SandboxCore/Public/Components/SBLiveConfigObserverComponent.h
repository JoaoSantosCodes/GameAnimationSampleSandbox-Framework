#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBLiveConfigTypes.h"
#include "SBLiveConfigObserverComponent.generated.h"

/**
 * Componente que observa recargas a quente de schemas de configuração e sincroniza parâmetros em tempo de execução
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOXCORE_API USBLiveConfigObserverComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBLiveConfigObserverComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	/** Schema observado por este componente */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|LiveConfig")
	FName WatchedSchema = TEXT("CombatBalance");

	/** Retorna a versão local observada */
	UFUNCTION(BlueprintPure, Category = "Sandbox|LiveConfig")
	int32 GetObservedVersion() const { return ObservedVersion; }

	/** Handler chamado ao receber notificação de hot-reload */
	UFUNCTION()
	void HandleSchemaHotReloaded(FName SchemaName, int32 NewVersion);

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|LiveConfig")
	FSBOnConfigSchemaHotReloaded OnSchemaUpdated;

private:
	int32 ObservedVersion = 1;
	void SyncTags();
};
