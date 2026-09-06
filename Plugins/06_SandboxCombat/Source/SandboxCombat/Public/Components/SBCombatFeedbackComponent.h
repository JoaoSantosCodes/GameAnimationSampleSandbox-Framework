#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBCombatFeedbackTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBCombatFeedbackComponent.generated.h"

class USBStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FSBOnHitStopTriggered, float /*Duration*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSBOnSlomoTriggered, float /*Dilation*/, float /*Duration*/);

/**
 * Componente para aplicação de feedback de impacto cinestésico, micro-pausas (Hit-Stop) e dilatação temporal (Slomo)
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBCombatFeedbackComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBCombatFeedbackComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Feedback")
	void ApplyHitStop(AActor* TargetActor, float Duration = 0.08f, float Dilation = 0.01f);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Feedback")
	void ApplyCombatFeedback(AActor* TargetActor, const FSBCombatFeedbackProfile& Profile);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Feedback")
	void TriggerSlomo(float TargetDilation = 0.2f, float Duration = 0.5f, bool bGlobal = false);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Feedback")
	void ResetHitStop();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Feedback")
	void ResetSlomo();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Feedback")
	bool IsHitStopActive() const { return bIsHitStopActive; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Feedback")
	bool IsSlomoActive() const { return bIsSlomoActive; }

	// Delegates
	FSBOnHitStopTriggered OnHitStopTriggered;
	FSBOnSlomoTriggered OnSlomoTriggered;

private:
	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;

	bool bIsHitStopActive = false;
	float HitStopRemainingTime = 0.0f;
	TWeakObjectPtr<AActor> HitStopTargetActor = nullptr;

	bool bIsSlomoActive = false;
	float SlomoRemainingTime = 0.0f;
	bool bIsSlomoGlobal = false;
};
