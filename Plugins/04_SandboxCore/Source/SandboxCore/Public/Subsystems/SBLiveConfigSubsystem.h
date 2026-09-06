#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBLiveConfigTypes.h"
#include "SBLiveConfigSubsystem.generated.h"

/**
 * Subsistema central de gerenciamento, reflexão e hot-reloading de schemas de configuração em tempo real
 */
UCLASS()
class SANDBOXCORE_API USBLiveConfigSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBLiveConfigSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Registra um novo schema de configuração */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LiveConfig")
	void RegisterSchema(FName SchemaName, const TMap<FName, float>& FloatProps, const TMap<FName, FString>& StringProps);

	/** Executa hot-reload de valores em um schema existente, incrementando a versão e notificando ouvintes */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LiveConfig")
	int32 HotReloadSchema(FName SchemaName, const TMap<FName, float>& NewFloatProps, const TMap<FName, FString>& NewStringProps);

	/** Consulta uma propriedade float */
	UFUNCTION(BlueprintPure, Category = "Sandbox|LiveConfig")
	float GetFloatConfig(FName SchemaName, FName PropName, float DefaultVal = 0.0f) const;

	/** Consulta uma propriedade textual */
	UFUNCTION(BlueprintPure, Category = "Sandbox|LiveConfig")
	FString GetStringConfig(FName SchemaName, FName PropName, const FString& DefaultVal = TEXT("")) const;

	/** Retorna a versão atual do schema */
	UFUNCTION(BlueprintPure, Category = "Sandbox|LiveConfig")
	int32 GetSchemaVersion(FName SchemaName) const;

	/** Retorna as métricas operacionais */
	UFUNCTION(BlueprintPure, Category = "Sandbox|LiveConfig")
	FSBLiveConfigMetrics GetMetrics() const;

	/** Reinicializa todos os schemas e métricas */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|LiveConfig")
	void ResetSubsystem();

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|LiveConfig")
	FSBOnConfigSchemaHotReloaded OnConfigSchemaHotReloaded;

private:
	TMap<FName, FSBLiveConfigSchema> RegisteredSchemas;

	int32 TotalHotReloadsExecuted = 0;
	int32 TotalPropertiesUpdated = 0;
};
