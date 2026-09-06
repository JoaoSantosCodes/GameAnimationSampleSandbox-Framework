#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBComboTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBComboComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBOnComboStepExecuted, int32, NodeId, const FSBComboNode&, Node, float, DamageMultiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBOnComboFinished, int32, TotalHits, float, FinalMultiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBOnComboReset);

/**
 * Componente responsável pelo sequenciamento de combos, ramificações de ataque e buffering de inputs
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBComboComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBComboComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Combo")
	void RegisterComboTree(const FSBComboTree& Tree);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Combo")
	bool ProcessComboInput(ESBComboInputType InputType);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Combo")
	void OpenComboWindow();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Combo")
	void CloseComboWindow();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Combo")
	void ResetCombo();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Combo")
	bool GetCurrentComboNode(FSBComboNode& OutNode) const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Combo")
	float GetCurrentDamageMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Combo")
	int32 GetComboCounter() const { return ComboCounter; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Combo")
	int32 GetCurrentNodeId() const { return CurrentNodeId; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Combo")
	bool IsWithinComboWindow() const { return bIsWindowOpen; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Combo")
	FSBOnComboStepExecuted OnComboStepExecuted;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Combo")
	FSBOnComboFinished OnComboFinished;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Combo")
	FSBOnComboReset OnComboReset;

private:
	void UpdateFinisherTag(bool bIsFinisher);

	UPROPERTY()
	FSBComboTree ActiveTree;

	int32 CurrentNodeId = INDEX_NONE;
	int32 ComboCounter = 0;
	float WindowTimer = 0.0f;
	bool bIsWindowOpen = false;
	bool bHasBufferedInput = false;
	ESBComboInputType BufferedInputType = ESBComboInputType::LightAttack;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
