#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Types/SBCommonTypes.h"
#include "Behaviors/SBGameplayBehavior.h"
#include "SBMovementBehavior.generated.h"

class USBMovementComponent;
class USBAttributeComponent;
class USBStateComponent;
class USBMovementBehaviorDefinition;

UCLASS(Abstract, Blueprintable, BlueprintType)
class SANDBOXCHARACTER_API USBMovementBehavior : public USBGameplayBehavior
{
	GENERATED_BODY()

public:
	USBMovementBehavior();

	// Inicializa e realiza o cacheamento de componentes para evitar lookups em runtime
	virtual void Initialize(USBBehaviorStackComponent* InStackComp, USBGameplayBehaviorDefinition* InDefinition) override;

	// Valida se o comportamento pode entrar na pilha ativa
	virtual bool CanEnter_Implementation(const FSBBehaviorContext& Context) const override;

	// Valida se o comportamento pode sair da pilha
	virtual bool CanExit_Implementation(const FSBBehaviorContext& Context) const override;

	virtual void Enter_Implementation(const FSBBehaviorContext& Context) override;

	virtual void Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context) override;

	virtual void Exit_Implementation(const FSBBehaviorContext& Context) override;

	int32 GetStackPriority() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Behavior")
	TObjectPtr<USBMovementComponent> MovementComponent = nullptr;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Behavior")
	TObjectPtr<USBMovementBehaviorDefinition> MovementDefinition = nullptr;

	USBMovementBehaviorDefinition* GetDefinition() const { return MovementDefinition; }

protected:

	// Caches dos componentes do Ator, inicializados de forma otimizada
	UPROPERTY(BlueprintReadOnly, Category = "Behavior|Cache")
	TObjectPtr<USBAttributeComponent> MovementAttributeComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Behavior|Cache")
	TObjectPtr<USBStateComponent> MovementStateComponent = nullptr;
};

UCLASS()
class SANDBOXCHARACTER_API USBMockReentrantBehavior : public USBMovementBehavior
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<USBMovementComponent> OwnerMC = nullptr;

	UPROPERTY()
	FGameplayTag TagToRequestOnExit;

	virtual void Exit_Implementation(const FSBBehaviorContext& Context) override;
};
