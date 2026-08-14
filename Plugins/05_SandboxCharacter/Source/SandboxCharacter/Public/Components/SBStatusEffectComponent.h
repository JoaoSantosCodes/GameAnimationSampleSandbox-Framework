#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "DataAssets/SBStatusEffectDefinition.h"
#include "SBStatusEffectComponent.generated.h"

class USBAttributeComponent;
class USBStateComponent;

USTRUCT(BlueprintType)
struct FSBStatusEffectEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffects")
	FGameplayTag EffectTag;

	UPROPERTY(BlueprintReadOnly, Category = "StatusEffects")
	float ExpiryTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StatusEffects")
	float LastPeriodTriggerTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StatusEffects")
	float Duration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StatusEffects")
	float Period = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StatusEffects")
	TObjectPtr<const USBStatusEffectDefinition> Definition = nullptr;
};

USTRUCT(BlueprintType)
struct FSBStatusEffectList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffects")
	TArray<FSBStatusEffectEntry> Entries;

	UPROPERTY(Transient)
	TWeakObjectPtr<class USBStatusEffectComponent> OwnerComponent = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FSBStatusEffectEntry, FSBStatusEffectList>(Entries, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits<FSBStatusEffectList> : public TStructOpsTypeTraitsBase2<FSBStatusEffectList>
{
	enum { WithNetDeltaSerializer = true };
};

/**
 * Componente autoritativo responsável por gerenciar a aplicação e expiração de Status Effects temporários e periódicos.
 */
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USBStatusEffectComponent();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sandbox|StatusEffects")
	void ApplyStatusEffect(const USBStatusEffectDefinition* Definition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sandbox|StatusEffects")
	void RemoveStatusEffect(FGameplayTag EffectTag);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|StatusEffects")
	bool HasStatusEffect(FGameplayTag EffectTag) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|StatusEffects")
	float GetEffectRemainingTime(FGameplayTag EffectTag) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Sandbox|StatusEffects")
	FSBStatusEffectList ActiveEffects;

	UPROPERTY(Transient)
	TObjectPtr<USBAttributeComponent> CachedAttributeComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USBStateComponent> CachedStateComponent = nullptr;
};
