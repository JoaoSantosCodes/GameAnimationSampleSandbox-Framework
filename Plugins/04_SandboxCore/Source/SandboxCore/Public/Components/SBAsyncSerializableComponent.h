// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBAsyncSerializationTypes.h"
#include "SBAsyncSerializableComponent.generated.h"

/**
 * Componente para captura e restauração determinística de snapshots binários por entidade
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SANDBOXCORE_API USBAsyncSerializableComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBAsyncSerializableComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;
	virtual void OnShutdown_Implementation() override;

	/** Captura o snapshot binário desta entidade */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Serialization")
	FSBAsyncSaveRecord CaptureSaveRecord(const TArray<uint8>& CustomPayload);

	/** Restaura o snapshot binário nesta entidade */
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Serialization")
	bool ApplyLoadRecord(const FSBAsyncSaveRecord& InRecord);

	/** GUID único e persistente da entidade */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Serialization")
	FGuid UniquePersistentGuid;

	/** Categoria de identificação do registro */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Serialization")
	FString SaveCategory = TEXT("DefaultEntity");

	/** Retorna os últimos dados binários restaurados ou capturados */
	UFUNCTION(BlueprintPure, Category = "Sandbox|Serialization")
	TArray<uint8> GetLastBinaryData() const { return LastBinaryData; }

private:
	TArray<uint8> LastBinaryData;
	void SyncTags();
};
