// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/SBBehaviorStackComponent.h"
#include "GameplayTagContainer.h"
#include "Types/SBCommonTypes.h"
#include "Subsystems/SBRPCRateLimiter.h"
#include "SBMovementComponent.generated.h"

class USBMovementBehavior;
class USBMovementBehaviorDefinition;
class USBMovementConfigDataAsset;
class USBBehaviorRegistry;
class USBMovementModifierAggregator;
class USBAttributeComponent;
class USBStateComponent;

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBMovementComponent : public USBBehaviorStackComponent
{
	GENERATED_BODY()

public:
	USBMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override;


	// API Pública
	virtual bool RequestBehavior(FGameplayTag BehaviorTag) override;



	USBMovementBehavior* GetCurrentBehavior() const;

	// RPCs de Sincronização de Rede (Multiplayer Ready)
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestBehavior(FGameplayTag BehaviorTag);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopBehavior(FGameplayTag BehaviorTag);

	UFUNCTION(Client, Reliable)
	void ClientStopBehavior(FGameplayTag BehaviorTag);

	// Carrega e ativa configurações de movimentação dinamicamente
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Movement")
	void LoadMovementConfig(USBMovementConfigDataAsset* NewConfig);

	// Aplica modificadores estruturados por tags de estatística
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Movement")
	void ApplyMovementModifiers(FGameplayTag SourceTag, const TArray<FSBModifierEntry>& Entries);

	// Remove todos os modificadores associados a uma fonte
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Movement")
	void RemoveMovementModifiers(FGameplayTag SourceTag);

	// Getter para o agregador de velocidade
	UFUNCTION(BlueprintPure, Category = "Sandbox|Movement")
	USBMovementModifierAggregator* GetSpeedModifierAggregator() const { return SpeedModifierAggregator; }

	// Autoriza uma realocação física (teleporte) no próximo tick de validação
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Movement|AntiCheat")
	void AuthorizeServerRelocation();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Movement|AntiCheat")
	float GetCalculatedMaxSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Stamina")
	bool ConsumeJumpStamina();

protected:
	// Data Asset contendo as configurações de comportamentos
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Movement")
	TObjectPtr<USBMovementConfigDataAsset> DefaultMovementConfig = nullptr;

	// Registro de comportamentos instanciados (Ownership)
	UPROPERTY(Transient)
	TObjectPtr<USBBehaviorRegistry> BehaviorRegistry = nullptr;

	// Agregador de modificadores de velocidade de caminhada/corrida/agachamento
	UPROPERTY(Transient)
	TObjectPtr<USBMovementModifierAggregator> SpeedModifierAggregator = nullptr;

	UPROPERTY(Transient)
	float LastStaminaConsumptionTime = 0.f;

	// Gancho Virtual para notificação de rede por domínio
	virtual void OnBehaviorEjected(FGameplayTag BehaviorTag, bool bSkipServerNotify, bool bSkipClientNotify) override;

	// Métodos Internos
	void EvaluateActivation();

	UPROPERTY(Transient)
	FSBRPCRateLimiter MovementRPCLimiter;

	UPROPERTY(Transient)
	FVector LastValidatedLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bHasLastValidatedLocation = false;

	UPROPERTY(Transient)
	bool bServerAuthorizedRelocation = false;

	mutable double LastLogDesyncTime = 0.0;

	UPROPERTY(Transient)
	float CachedCmcMaxWalkSpeed = 0.f;

	UPROPERTY(Transient)
	float CachedAttrBaseSpeed = 0.f;

	UPROPERTY(Transient)
	bool bHasInitializedCachedSpeeds = false;

	/**
	 * Resolvem os componentes irmaos do dono memorizando o resultado, para que o caminho de
	 * tick nao repita FindComponentByClass a cada frame.
	 *
	 * Sao const e usam membros mutable porque GetCalculatedMaxSpeed() e const e esta no
	 * caminho critico. TWeakObjectPtr se anula sozinho quando o componente e destruido, entao
	 * a revalidacao a cada acesso forca nova busca em vez de devolver ponteiro pendente.
	 */
	USBAttributeComponent* GetAttributeComponent() const;
	USBStateComponent* GetStateComponent() const;

	mutable TWeakObjectPtr<USBAttributeComponent> CachedAttributeComponent;
	mutable TWeakObjectPtr<USBStateComponent> CachedStateComponent;

public:
	// Helper para encontrar instâncias tipadas
	USBMovementBehavior* FindAvailableBehaviorByTag(FGameplayTag Tag) const;
	USBMovementBehavior* FindActiveBehaviorByTag(FGameplayTag Tag) const;
};
