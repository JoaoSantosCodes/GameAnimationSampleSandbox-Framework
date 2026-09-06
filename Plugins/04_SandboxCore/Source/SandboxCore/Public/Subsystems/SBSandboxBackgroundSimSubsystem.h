#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Interfaces/SBBackgroundSimInterface.h"
#include "Interfaces/SBSaveInterface.h"
#include "SBSandboxBackgroundSimSubsystem.generated.h"

UCLASS(BlueprintType)
class SANDBOXCORE_API USBSandboxBackgroundSimSubsystem : public UTickableWorldSubsystem, public ISBSaveInterface
{
	GENERATED_BODY()

public:
	USBSandboxBackgroundSimSubsystem();

	// USubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// UTickableWorldSubsystem implementation
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return bIsTickEnabled; }

	// ISBSaveInterface implementation
	virtual bool SaveComponentData_Implementation(UObject* SavePayload) override;
	virtual bool LoadComponentData_Implementation(UObject* SavePayload) override;
	virtual int32 GetSavePriority_Implementation() const override { return 100; }

	// API de Simulação
	UFUNCTION(BlueprintCallable, Category = "Sandbox|BackgroundSim")
	void RegisterSimulatedEntity(const FSBSimulatedEntityData& EntityData);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|BackgroundSim")
	void UnregisterSimulatedEntity(FGuid EntityId);

	UFUNCTION(BlueprintPure, Category = "Sandbox|BackgroundSim")
	bool GetSimulatedEntityData(FGuid EntityId, FSBSimulatedEntityData& OutData) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|BackgroundSim")
	void ClearAllSimulations();

	// Permite simular o avanço do tempo manualmente (útil para testes de Catch-up)
	UFUNCTION(BlueprintCallable, Category = "Sandbox|BackgroundSim")
	void ForceAdvanceTime(float DeltaSeconds);

private:
	// Entidades sob simulação ativa (unloaded)
	UPROPERTY(SaveGame)
	TMap<FGuid, FSBSimulatedEntityData> ActiveSimulations;

	UPROPERTY(SaveGame)
	int64 LastSavedTimestamp = 0;

	// Intervalo de tick em segundos (para otimizar CPU, padrão 1.0s)
	UPROPERTY(EditDefaultsOnly, Category = "BackgroundSim")
	float BackgroundTickInterval = 1.0f;

	float TimeSinceLastTick = 0.0f;
	bool bIsTickEnabled = true;
};
