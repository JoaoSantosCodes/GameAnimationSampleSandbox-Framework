#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBDefenseTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBDefenseComponent.generated.h"

class USBAttributeComponent;
class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBOnBlockSuccess, float, OriginalDamage, float, MitigatedDamage, AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnParrySuccess, AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnGuardBroken, AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBOnCounterAttackWindowExpired);

/**
 * Componente responsável pelo sistema defensivo: Bloqueio, Parry/Perfect Block, Quebra de Guarda e Contra-Ataques
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBDefenseComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBDefenseComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Defense")
	void StartBlocking();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Defense")
	void StopBlocking();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Defense")
	ESBBlockResult ProcessIncomingDamage(float InDamage, AActor* Attacker, float& OutMitigatedDamage);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Defense")
	float ConsumeCounterAttack();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Defense")
	bool IsBlocking() const { return bIsBlocking; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Defense")
	bool IsParryActive() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Defense")
	bool IsCounterAttackReady() const { return CounterAttackTimer > 0.0f; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Combat|Defense")
	FSBDefenseSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Defense")
	FSBOnBlockSuccess OnBlockSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Defense")
	FSBOnParrySuccess OnParrySuccess;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Defense")
	FSBOnGuardBroken OnGuardBroken;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Defense")
	FSBOnCounterAttackWindowExpired OnCounterAttackWindowExpired;

private:
	bool bIsBlocking = false;
	float BlockTime = 0.0f;
	float CounterAttackTimer = 0.0f;

	UPROPERTY()
	TWeakObjectPtr<USBAttributeComponent> CachedAttributeComp;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
