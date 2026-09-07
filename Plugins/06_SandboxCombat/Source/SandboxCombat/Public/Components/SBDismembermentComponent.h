// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Types/SBDismembermentTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBDismembermentComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FSBOnLimbSevered, ESBLimbType /*LimbType*/, const FVector& /*Impulse*/);

/**
 * Componente para gerenciamento de amputação dinâmica de membros, ocultação óssea e física de partes decepadas
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBDismembermentComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBDismembermentComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	// Dismemberment API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Dismemberment")
	void RegisterLimbDefinition(const FSBLimbDismemberDefinition& Def);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Dismemberment")
	bool SeverLimb(const FSBSeverLimbRequest& Request, USkeletalMeshComponent* MeshComp = nullptr);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Dismemberment")
	bool IsLimbSevered(ESBLimbType LimbType) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Dismemberment")
	int32 GetSeveredLimbCount() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Dismemberment")
	bool GetLimbDefinition(ESBLimbType LimbType, FSBLimbDismemberDefinition& OutDef) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Dismemberment")
	void ResetDismemberment(USkeletalMeshComponent* MeshComp = nullptr);

	// Delegates
	FSBOnLimbSevered OnLimbSevered;

private:
	UPROPERTY()
	TMap<ESBLimbType, FSBLimbDismemberDefinition> RegisteredLimbs;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
