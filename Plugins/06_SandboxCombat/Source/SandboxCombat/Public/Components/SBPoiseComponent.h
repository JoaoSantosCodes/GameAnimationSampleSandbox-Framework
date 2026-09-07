// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SBPoiseTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "SBPoiseComponent.generated.h"

class USBStateComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnPoiseDamaged, float, CurrentPoise);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnPoiseBroken, const FSBHitReactionResult&, ReactionResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSBOnPoiseRecovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSBOnHitReactionTriggered, const FSBHitReactionResult&, ReactionResult);

/**
 * Componente responsável pelo sistema de postura (Poise), super armor e cálculo direcional de reações de impacto
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCOMBAT_API USBPoiseComponent : public UActorComponent, public ISBComponentInterface
{
	GENERATED_BODY()

public:
	USBPoiseComponent();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Poise")
	void ReceivePoiseDamage(float Damage, const FVector& HitLocation, AActor* InstigatorActor, FSBHitReactionResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Poise")
	void SetSuperArmor(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Combat|Poise")
	void ResetPoise();

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Poise")
	float GetCurrentPoise() const { return CurrentPoise; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Poise")
	float GetMaxPoise() const { return Settings.MaxPoise; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Poise")
	bool IsPoiseBroken() const { return bIsPoiseBroken; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Poise")
	bool HasSuperArmor() const { return Settings.bHasSuperArmor; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Combat|Poise")
	ESBHitReactionDirection CalculateHitDirection(const FVector& HitLocation) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sandbox|Combat|Poise")
	FSBPoiseSettings Settings;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Poise")
	FSBOnPoiseDamaged OnPoiseDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Poise")
	FSBOnPoiseBroken OnPoiseBroken;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Poise")
	FSBOnPoiseRecovered OnPoiseRecovered;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Combat|Poise")
	FSBOnHitReactionTriggered OnHitReactionTriggered;

private:
	float CurrentPoise = 100.0f;
	float RegenDelayTimer = 0.0f;
	float StaggerTimer = 0.0f;
	bool bIsPoiseBroken = false;

	UPROPERTY()
	TWeakObjectPtr<USBStateComponent> CachedStateComp;
};
