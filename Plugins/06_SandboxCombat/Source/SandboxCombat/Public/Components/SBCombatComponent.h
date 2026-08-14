#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SBComponentInterface.h"
#include "Types/SBCommonTypes.h"
#include "Components/SBBehaviorStackComponent.h"
#include "Subsystems/SBRPCRateLimiter.h"
#include "SBCombatComponent.generated.h"

class USBWeaponBehavior;
class USBWeaponBehaviorDefinition;
class USBAttributeComponent;
class USBStateComponent;

USTRUCT(BlueprintType)
struct FSBWeaponConfigEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<USBWeaponBehavior> BehaviorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USBWeaponBehaviorDefinition> DefinitionAsset;
};

UCLASS(BlueprintType)
class SANDBOXCOMBAT_API USBCombatConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<FSBWeaponConfigEntry> ConfiguredWeapons;
};


UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBCombatComponent : public USBBehaviorStackComponent
{
	GENERATED_BODY()

public:
	USBCombatComponent();

	// ISBDebugInterface
	virtual void GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const override;

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;


	// API Pública
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat")
	bool RequestWeaponBehavior(FGameplayTag BehaviorTag, int32 PredictionId = 0);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat")
	void StopWeaponBehavior(FGameplayTag BehaviorTag, bool bSkipServerNotify = false);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat")
	bool HasWeaponBehavior(FGameplayTag BehaviorTag) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat")
	void LoadCombatConfig(USBCombatConfigDataAsset* NewConfig);

	// RPCs de Disparo e Validação de Rede
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestFire(FGameplayTag BehaviorTag, int32 PredictionId);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFire(FGameplayTag BehaviorTag);

	UFUNCTION(Client, Reliable)
	void ClientRollbackFire(FGameplayTag BehaviorTag, int32 PredictionId);

	// Getters de Teste
	TArray<TObjectPtr<USBWeaponBehavior>> GetActiveWeapons() const;
	TArray<TObjectPtr<USBWeaponBehavior>> GetAvailableWeapons() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Combat")
	TObjectPtr<USBCombatConfigDataAsset> DefaultCombatConfig = nullptr;

	UPROPERTY(Transient)
	TMap<FGameplayTag, float> LastExecutionTimes;

	UPROPERTY(Transient)
	TObjectPtr<USBAttributeComponent> CachedAttributeComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USBStateComponent> CachedStateComponent = nullptr;

	// Gancho Virtual para notificação de rede por domínio
	virtual void OnBehaviorEjected(FGameplayTag BehaviorTag, bool bSkipServerNotify, bool bSkipClientNotify) override;

	USBWeaponBehavior* FindAvailableWeaponByTag(FGameplayTag Tag) const;
	USBWeaponBehavior* FindActiveWeaponByTag(FGameplayTag Tag) const;

private:
	UFUNCTION()
	void OnItemEquipped(FGameplayTag EventTag, UObject* Payload);

	UFUNCTION()
	void OnItemUnequipped(FGameplayTag EventTag, UObject* Payload);

	UPROPERTY(Transient)
	FSBRPCRateLimiter CombatRPCLimiter;
};

UCLASS()
class SANDBOXCOMBAT_API USBTestCombatComponent : public USBCombatComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayTag LastRollbackTag;
	
	UPROPERTY()
	int32 LastRollbackPredictionId = 0;

	virtual void ClientRollbackFire_Implementation(FGameplayTag BehaviorTag, int32 PredictionId) override;
};
