#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SBComponentInterface.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "Interfaces/SBDebugInterface.h"
#include "SBStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSBStateChangedSignature, FGameplayTag, StateTag, bool, bAdded);

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBStateComponent : public UGameFrameworkComponent, public ISBComponentInterface, public ISBStateComponentInterface, public ISBDebugInterface
{
	GENERATED_BODY()

public:
	USBStateComponent();

	// ISBDebugInterface
	virtual void GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const override;

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override {}
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	UFUNCTION(BlueprintCallable, Category = "Sandbox|States")
	void AddTag(FGameplayTag StateTag);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|States")
	void RemoveTag(FGameplayTag StateTag);

	bool HasTag(FGameplayTag StateTag) const;

	bool HasAny(FGameplayTagContainer TagsContainer) const;

	bool HasAll(FGameplayTagContainer TagsContainer) const;

	// ISBStateComponentInterface
	virtual bool HasTag_Implementation(FGameplayTag StateTag) const override { return HasTag(StateTag); }
	virtual bool HasAny_Implementation(FGameplayTagContainer TagsContainer) const override { return HasAny(TagsContainer); }
	virtual bool HasAll_Implementation(FGameplayTagContainer TagsContainer) const override { return HasAll(TagsContainer); }

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|States")
	FSBStateChangedSignature OnStateChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_ActiveStateTags, Category = "Sandbox|States")
	FGameplayTagContainer ActiveStateTags;

	// Tags preditas localmente pelo cliente autônomo (não replicadas)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Sandbox|States")
	FGameplayTagContainer PredictedStateTags;

	UFUNCTION()
	void OnRep_ActiveStateTags(const FGameplayTagContainer& OldTags);
};

UCLASS()
class SANDBOXCHARACTER_API USBTestStateComponent : public USBStateComponent
{
	GENERATED_BODY()

public:
	const FGameplayTagContainer& GetPredictedStateTags() const { return PredictedStateTags; }
	const FGameplayTagContainer& GetActiveStateTags() const { return ActiveStateTags; }
};
