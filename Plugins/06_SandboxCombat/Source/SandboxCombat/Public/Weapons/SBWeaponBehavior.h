#pragma once

#include "Behaviors/SBGameplayBehavior.h"
#include "SBWeaponBehavior.generated.h"

class USBCombatComponent;
class USBAttributeComponent;
class USBStateComponent;
class USBWeaponBehaviorDefinition;

UCLASS(Abstract, Blueprintable, BlueprintType)
class SANDBOXCOMBAT_API USBWeaponBehavior : public USBGameplayBehavior
{
	GENERATED_BODY()

public:
	USBWeaponBehavior();

	// Inicializa e realiza o cacheamento de componentes para evitar lookups em runtime
	virtual void Initialize(USBBehaviorStackComponent* InStackComp, USBGameplayBehaviorDefinition* InDefinition) override;

	// Valida se o comportamento de arma pode entrar na pilha ativa
	virtual bool CanEnter_Implementation(const FSBBehaviorContext& Context) const override;

	// Valida se o comportamento de arma pode sair da pilha
	virtual bool CanExit_Implementation(const FSBBehaviorContext& Context) const override;

	virtual void Enter_Implementation(const FSBBehaviorContext& Context) override;

	virtual void Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context) override;

	virtual void Exit_Implementation(const FSBBehaviorContext& Context) override;

	int32 GetStackPriority() const;

	USBWeaponBehaviorDefinition* GetDefinition() const { return WeaponDefinition; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Behavior")
	TObjectPtr<USBCombatComponent> CombatComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Behavior")
	TObjectPtr<USBWeaponBehaviorDefinition> WeaponDefinition = nullptr;

	// Caches dos componentes do Ator, inicializados de forma otimizada
	UPROPERTY(BlueprintReadOnly, Category = "Behavior|Cache")
	TObjectPtr<USBAttributeComponent> CombatAttributeComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Behavior|Cache")
	TObjectPtr<USBStateComponent> CombatStateComponent = nullptr;
};
