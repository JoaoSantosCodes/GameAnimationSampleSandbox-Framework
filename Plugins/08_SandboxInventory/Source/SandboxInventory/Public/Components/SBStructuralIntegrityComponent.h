// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBStructuralTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBStructuralIntegrityComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBStructuralStabilityChanged, ESBStructuralStabilityState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBStructuralLoadChanged, float, CurrentLoad, float, MaxCapacity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBStructuralCollapse);

/**
 * Componente de cálculo de integridade estrutural, estresse de carga e colapso físico de construções
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXINVENTORY_API USBStructuralIntegrityComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBStructuralIntegrityComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Structural Integrity API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Structural")
	void SetGroundAnchor(bool bAnchor);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Structural")
	void RegisterNeighborPiece(USBStructuralIntegrityComponent* NeighborComp);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Structural")
	void UnregisterNeighborPiece(USBStructuralIntegrityComponent* NeighborComp);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Structural")
	void AddSupportedLoad(float LoadMass);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Structural")
	void RemoveSupportedLoad(float LoadMass);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Structural")
	void RecalculateIntegrity(TArray<USBStructuralIntegrityComponent*>& Visited);

	/** Coleta toda a rede conectada a partir desta peca (busca em largura). */
	void GatherStructuralNetwork(TArray<USBStructuralIntegrityComponent*>& OutNetwork);

	/** Aplica estado de estabilidade, carga e tags a partir da distancia ja calculada. */
	void ApplyStabilityState();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Building|Structural")
	void TriggerStructuralCollapse();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Structural")
	FSBStructuralNodeData GetStructuralData() const { return StructuralData; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Structural")
	ESBStructuralStabilityState GetStabilityState() const { return StructuralData.StabilityState; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Building|Structural")
	bool IsSupported() const { return StructuralData.bIsGroundAnchor || (StructuralData.DistanceFromAnchor != INDEX_NONE && StructuralData.StabilityState != ESBStructuralStabilityState::Collapsing); }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Building|Structural")
	FSBStructuralSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Structural")
	FSBStructuralStabilityChanged OnStructuralStabilityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Structural")
	FSBStructuralLoadChanged OnStructuralLoadChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Building|Structural")
	FSBStructuralCollapse OnStructuralCollapse;

private:
	void SyncStructuralTags();

	UPROPERTY()
	FSBStructuralNodeData StructuralData;

	UPROPERTY()
	TArray<TWeakObjectPtr<USBStructuralIntegrityComponent>> ConnectedNeighbors;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
