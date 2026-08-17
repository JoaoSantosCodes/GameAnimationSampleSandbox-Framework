#pragma once

#include "CoreMinimal.h"
#include "Components/SBBehaviorStackComponent.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Subsystems/SBRPCRateLimiter.h"
#include "SBAbilityComponent.generated.h"

class USBAbility;
class UInputComponent;

USTRUCT(BlueprintType)
struct FSBCooldownEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	FGameplayTag AbilityTag;

	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	float ExpiryTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FSBCooldownList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	TArray<FSBCooldownEntry> Entries;

	UPROPERTY(Transient)
	TWeakObjectPtr<class USBAbilityComponent> OwnerComponent = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FSBCooldownEntry, FSBCooldownList>(Entries, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits<FSBCooldownList> : public TStructOpsTypeTraitsBase2<FSBCooldownList>
{
	enum { WithNetDeltaSerializer = true };
};

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBAbilityComponent : public USBBehaviorStackComponent
{
	GENERATED_BODY()

	friend class FSBAbilityTestsSpec;

public:
	USBAbilityComponent();

	// ISBDebugInterface
	virtual void GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const override;

	virtual void OnInitialize_Implementation() override;
	virtual void OnPostInitialize_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Abilities")
	USBAbility* GrantAbility(TSubclassOf<USBAbility> AbilityClass, USBGameplayBehaviorDefinition* Definition = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Abilities")
	bool ActivateAbilityByTag(FGameplayTag AbilityTag);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Abilities")
	void EndAbilityByTag(FGameplayTag AbilityTag);

	void BindInputActions(UInputComponent* PlayerInputComponent);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Abilities")
	bool IsAbilityOnCooldown(FGameplayTag AbilityTag) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Abilities")
	float GetRemainingCooldownTime(FGameplayTag AbilityTag) const;

	virtual bool RequestBehavior(FGameplayTag BehaviorTag) override;
	virtual void StopBehavior(FGameplayTag BehaviorTag, bool bSkipServerNotify = false, bool bSkipClientNotify = false) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestBehavior(FGameplayTag BehaviorTag, int32 PredictionId);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopBehavior(FGameplayTag BehaviorTag);

	UFUNCTION(Client, Reliable)
	void ClientStopBehavior(FGameplayTag BehaviorTag);

	UFUNCTION(Client, Reliable)
	void ClientRollbackAbility(FGameplayTag BehaviorTag, int32 PredictionId);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void OnBehaviorEjected(FGameplayTag BehaviorTag, bool bSkipServerNotify, bool bSkipClientNotify) override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Sandbox|Abilities")
	FSBCooldownList CooldownsList;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FGameplayTag> InputToAbilityMap;

	int32 LocalPredictionId = 0;
	int32 CurrentServerPredictionId = 0;

	UPROPERTY(Transient)
	FSBRPCRateLimiter AbilityRPCLimiter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Abilities")
	float ManaRegenRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Abilities")
	float ManaRegenDelay = 2.0f;

	UPROPERTY(Transient)
	float LastManaConsumptionTime = 0.0f;

	void Input_AbilityInputPressed(FGameplayTag InputTag);
	void Input_AbilityInputReleased(FGameplayTag InputTag);
};
