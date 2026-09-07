// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interfaces/SBBackgroundSimInterface.h"
#include "SBResourceNode.generated.h"

UCLASS(Blueprintable, BlueprintType)
class SANDBOXINVENTORY_API ASBResourceNode : public AActor, public ISBBackgroundSimInterface
{
	GENERATED_BODY()

public:
	ASBResourceNode();

	// ISBBackgroundSimInterface implementation
	virtual void PrepareForBackgroundSim_Implementation(FSBSimulatedEntityData& OutData) override;
	virtual void ResumeFromBackgroundSim_Implementation(const FSBSimulatedEntityData& InData) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Sobrescreve a função de dano padrão da Unreal
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "Sandbox|Resource")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Resource")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Resource")
	bool IsDepleted() const { return bIsDepleted; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Resource")
	float GetRespawnTime() const { return RespawnTime; }

	FTimerHandle GetRespawnTimerHandle() const { return RespawnTimerHandle; }

	// Método auxiliar de testes para forçar a simulação de respawn
	void DebugForceRespawn() { HandleRespawn(); }

	// Método auxiliar de testes para injetar a tag da ferramenta ativa
	void SetDebugActiveToolTag(FGameplayTag NewTag) { DebugActiveToolTag = NewTag; }

private:
	FGameplayTag DebugActiveToolTag;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// HP máximo do nó de recurso
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	float MaxHealth = 100.0f;

	// HP atual replicado do recurso
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Resource")
	float Health;

	// Tag de ferramenta necessária para coletar (ex: Tool.Pickaxe ou Tool.Axe)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	FGameplayTag RequiredToolTag;

	// Tabela de Loot que determina os itens concedidos
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	TObjectPtr<class USBLootTableDataAsset> LootTable = nullptr;

	// Tempo de respawn em segundos após esgotado (0.0 = sem respawn)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	float RespawnTime = 10.0f;

	// Malha visível do recurso
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> StaticMeshComp;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	// Lógica executada quando o recurso zera o HP
	void HandleDepletion(AActor* DamageCauser);

	// Lógica executada para restaurar/respawnar o recurso
	void HandleRespawn();

private:
	UPROPERTY(ReplicatedUsing = OnRep_IsDepleted)
	bool bIsDepleted = false;

	UFUNCTION()
	void OnRep_IsDepleted();

	FTimerHandle RespawnTimerHandle;
};
