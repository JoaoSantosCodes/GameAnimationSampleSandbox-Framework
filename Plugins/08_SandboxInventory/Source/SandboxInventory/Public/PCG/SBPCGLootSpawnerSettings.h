// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "PCGElement.h"
#include "DataAssets/SBLootTableDataAsset.h"
#include "SBPCGLootSpawnerSettings.generated.h"

UCLASS(BlueprintType, ClassGroup = (Procedural))
class SANDBOXINVENTORY_API USBPCGLootSpawnerSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	// Tabela de loot usada para selecionar quais itens/recursos spawnam em cada ponto
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TObjectPtr<USBLootTableDataAsset> LootTable = nullptr;

	// Dicionário mapeando itens gerados para classes de Atores a serem instanciados
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TMap<TObjectPtr<USBItemDefinition>, TSubclassOf<AActor>> ItemToActorMap;

	// Classe padrão para spawnar caso a tabela de loot não retorne itens cadastrados
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSubclassOf<AActor> DefaultSpawnClass = nullptr;

	// Nome do atributo de metadata a ser injetado (ex: "ActorClass")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FName ClassAttributeName = TEXT("ActorClass");

	// ~UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName("SandboxLootSpawner"); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("USBPCGLootSpawnerSettings", "NodeTitle", "Sandbox Loot Spawner"); }
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("USBPCGLootSpawnerSettings", "NodeTooltip", "Assigns actor classes to points based on Sandbox Loot Table roll results."); }
#endif
	virtual FString GetAdditionalTitleInformation() const override;
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

protected:
	virtual FPCGElementPtr CreateElement() const override;
};

class FSBPCGLootSpawnerElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};
