// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "SBLiveConfigTypes.generated.h"

/**
 * Propriedade tipada de configuração dinâmica
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLiveConfigProperty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	FName PropertyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	float FloatValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	bool bBoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	int32 SchemaVersion = 1;

	FSBLiveConfigProperty() = default;

	FSBLiveConfigProperty(FName InName, float InFloat, int32 InVer = 1)
		: PropertyName(InName), FloatValue(InFloat), SchemaVersion(InVer) {}

	FSBLiveConfigProperty(FName InName, const FString& InStr, int32 InVer = 1)
		: PropertyName(InName), StringValue(InStr), SchemaVersion(InVer) {}
};

/**
 * Schema de configuração com controle de versão e recarga a quente
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLiveConfigSchema
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	FName SchemaName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	int32 Version = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	TMap<FName, FSBLiveConfigProperty> Properties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	int64 LastReloadTicks = 0;
};

/**
 * Métricas do subsistema de live config
 */
USTRUCT(BlueprintType)
struct SANDBOXCOMMON_API FSBLiveConfigMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	int32 TotalSchemasRegistered = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	int32 TotalHotReloadsExecuted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	int32 TotalPropertiesUpdated = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveConfig")
	int32 TotalSubscribedListeners = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnConfigSchemaHotReloaded, FName, SchemaName, int32, NewVersion);
