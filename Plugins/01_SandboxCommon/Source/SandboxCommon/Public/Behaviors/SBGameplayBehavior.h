#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Types/SBCommonTypes.h"
#include "SBGameplayBehavior.generated.h"

class USBBehaviorStackComponent;
class USBGameplayBehaviorDefinition;

UCLASS(Abstract, Blueprintable, BlueprintType)
class SANDBOXCOMMON_API USBGameplayBehavior : public UObject
{
	GENERATED_BODY()

public:
	USBGameplayBehavior();

	// Inicializa e realiza o cacheamento de componentes para evitar lookups em runtime
	virtual void Initialize(USBBehaviorStackComponent* InStackComp, USBGameplayBehaviorDefinition* InDefinition);

	// Valida se o comportamento pode entrar na pilha ativa
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox|Behavior")
	bool CanEnter(const FSBBehaviorContext& Context) const;

	// Valida se o comportamento pode sair da pilha
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sandbox|Behavior")
	bool CanExit(const FSBBehaviorContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Sandbox|Behavior")
	void Enter(const FSBBehaviorContext& Context);

	UFUNCTION(BlueprintNativeEvent, Category = "Sandbox|Behavior")
	void Update(float DeltaTime, const FSBBehaviorContext& Context);

	UFUNCTION(BlueprintNativeEvent, Category = "Sandbox|Behavior")
	void Exit(const FSBBehaviorContext& Context);

	int32 GetStackPriority() const;
	USBGameplayBehaviorDefinition* GetDefinition() const { return Definition; }
	FGameplayTag GetBehaviorTag() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Behavior")
	TObjectPtr<USBBehaviorStackComponent> StackComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Behavior")
	TObjectPtr<USBGameplayBehaviorDefinition> Definition = nullptr;

	// Caches genéricos de componentes do Ator, inicializados na inicialização
	UPROPERTY(BlueprintReadOnly, Category = "Behavior|Cache")
	TObjectPtr<UActorComponent> CachedAttributeComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Behavior|Cache")
	TObjectPtr<UActorComponent> CachedStateComponent = nullptr;
};
