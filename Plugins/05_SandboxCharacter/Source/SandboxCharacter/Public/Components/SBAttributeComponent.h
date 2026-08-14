#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "GameplayTagContainer.h"
#include "Types/SBCommonTypes.h"
#include "Interfaces/SBComponentInterface.h"
#include "Interfaces/SBSaveInterface.h"
#include "Interfaces/SBDebugInterface.h"
#include "SBAttributeComponent.generated.h"

USTRUCT(BlueprintType)
struct FSBPendingAttributePrediction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayTag AttributeTag;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float Amount = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	int32 PredictionId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct FSBConfirmedPredictionEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayTag AttributeTag;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	int32 ConfirmedPredictionId = 0;
};

USTRUCT(BlueprintType)
struct FSBAttributeReplicationEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayTag Tag;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FSBAttribute Attribute;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSBAttributeChangedSignature, FGameplayTag, AttributeTag, float, NewValue, float, OldValue, AActor*, Instigator);

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBAttributeComponent : public UGameFrameworkComponent, public ISBComponentInterface, public ISBSaveInterface, public ISBDebugInterface
{
	GENERATED_BODY()

public:
	USBAttributeComponent();

	// ISBDebugInterface
	virtual void GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const override;

	UFUNCTION()
	void OnRep_ConfirmedPredictions();

	UFUNCTION()
	void OnRep_ReplicatedAttributes();

	// ISBComponentInterface
	virtual void OnComponentCreated_Implementation() override {}
	virtual void OnPreInitialize_Implementation() override {}
	virtual void OnInitialize_Implementation() override {}
	virtual void OnPostInitialize_Implementation() override {}
	virtual void OnReady_Implementation() override {}
	virtual void OnShutdown_Implementation() override {}

	// ISBSaveInterface
	virtual bool SaveComponentData_Implementation(UObject* SavePayload) override;
	virtual bool LoadComponentData_Implementation(UObject* SavePayload) override;
	virtual int32 GetSavePriority_Implementation() const override { return 100; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core API
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Attributes")
	void RegisterAttribute(FGameplayTag AttributeTag, FSBAttribute InitialValue);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Attributes")
	bool GetAttribute(FGameplayTag AttributeTag, FSBAttribute& OutAttribute) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Attributes")
	float GetAttributeValue(FGameplayTag AttributeTag) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Attributes")
	void SetAttributeBaseValue(FGameplayTag AttributeTag, float NewValue);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Attributes")
	void ApplyModifier(FGameplayTag AttributeTag, FSBAttributeModifier Modifier);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Attributes")
	void RemoveModifiersBySource(FGameplayTag AttributeTag, FGameplayTag SourceTag);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Attributes")
	bool TryConsumeAttribute(FGameplayTag AttributeTag, float Amount, AActor* Instigator, int32 PredictionId = 0);

	// Confirmação de consumo pelo servidor (Server-only)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sandbox|Attributes")
	void ConfirmPrediction(FGameplayTag AttributeTag, int32 PredictionId);

	// RPC de Rollback para o cliente em caso de rejeição
	UFUNCTION(Client, Reliable)
	void ClientRollbackPrediction(FGameplayTag AttributeTag, int32 PredictionId);

	// Getters para testes
	const TArray<FSBConfirmedPredictionEntry>& GetConfirmedPredictions() const { return ConfirmedPredictions; }
	const TArray<FSBPendingAttributePrediction>& GetPendingPredictions() const { return PendingPredictions; }

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Attributes")
	FSBAttributeChangedSignature OnAttributeChanged;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleAttributeChangedInternal(FGameplayTag AttributeTag, float NewValue, float OldValue, AActor* Instigator);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Sandbox|Attributes")
	TMap<FGameplayTag, FSBAttribute> AttributesMap;

	// Array replicado de confirmações de predição por atributo (Upsert)
	UPROPERTY(ReplicatedUsing = OnRep_ConfirmedPredictions)
	TArray<FSBConfirmedPredictionEntry> ConfirmedPredictions;

	// Array replicado de atributos para sincronização de rede
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedAttributes)
	TArray<FSBAttributeReplicationEntry> ReplicatedAttributes;

	void UpdateReplicatedAttribute(FGameplayTag Tag, const FSBAttribute& Attr);

	void ModifyAttributeBaseValue(FGameplayTag Tag, float NewValue, AActor* Instigator = nullptr);

	void CleanPendingPredictionsAndNotify();

	// Fila local de transações pendentes no cliente
	UPROPERTY(Transient)
	TArray<FSBPendingAttributePrediction> PendingPredictions;

	TMap<FGameplayTag, TArray<FSBAttributeModifier>> ActiveModifiers;

	void UpdateRegeneration(float DeltaTime);
	void UpdateModifiers(float DeltaTime);
	float CalculateValueWithModifiers(FGameplayTag AttributeTag, float BaseVal) const;
};
