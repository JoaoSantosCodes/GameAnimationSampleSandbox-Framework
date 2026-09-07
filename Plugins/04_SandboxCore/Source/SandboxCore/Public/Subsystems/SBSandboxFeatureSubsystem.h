// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "SBSandboxFeatureSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnFeatureToggled, FGameplayTag, FeatureTag, bool, bEnabled);

UCLASS(BlueprintType)
class SANDBOXCORE_API USBSandboxFeaturePayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Features")
	FGameplayTag FeatureTag;

	UPROPERTY(BlueprintReadOnly, Category = "Features")
	bool bEnabled = false;
};

/**
 * Subsistema global para gerenciar o estado ativo de Features e Capabilities do jogo em tempo de execução.
 */
UCLASS(BlueprintType)
class SANDBOXCORE_API USBSandboxFeatureSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USBSandboxFeatureSubsystem();

	// Verifica se uma feature global está ativa
	UFUNCTION(BlueprintPure, Category = "Sandbox|Features")
	bool IsFeatureEnabled(FGameplayTag FeatureTag) const;

	// Habilita uma feature globalmente e notifica ouvintes
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Features")
	void EnableFeature(FGameplayTag FeatureTag);

	// Desabilita uma feature globalmente e notifica ouvintes
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Features")
	void DisableFeature(FGameplayTag FeatureTag);

	// Evento disparado quando qualquer feature é ligada ou desligada
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Features")
	FSBOnFeatureToggled OnFeatureToggled;

protected:
	UPROPERTY(Transient)
	FGameplayTagContainer ActiveFeatures;
};
