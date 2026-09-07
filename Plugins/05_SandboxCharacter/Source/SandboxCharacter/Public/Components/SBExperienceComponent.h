// Copyright 2026 João Santos. All Rights Reserved.
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SBExperienceComponent.generated.h"

/**
 * Estrutura para configurar os requisitos de XP por nível via DataTable.
 */
USTRUCT(BlueprintType)
struct FRequiredXPRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	int32 RequiredXP = 100;
};

// Delegate para notificação de alteração de XP (não-dinâmico para suporte a lambdas em C++)
DECLARE_MULTICAST_DELEGATE_ThreeParams(FSBExperienceChangedSignature, int32 /* NewXP */, int32 /* DeltaXP */, int32 /* RequiredXP */);

// Delegate para notificação de Level Up (não-dinâmico)
DECLARE_MULTICAST_DELEGATE_OneParam(FSBLevelUpSignature, int32 /* NewLevel */);

/**
 * Componente que gerencia o nível, experiência e progressão dos personagens.
 */
UCLASS(ClassGroup=(Sandbox), meta=(BlueprintSpawnableComponent))
class SANDBOXCHARACTER_API USBExperienceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USBExperienceComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Adiciona pontos de experiência de forma autoritativa no servidor */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sandbox|Experience")
	void AddExperience(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Experience")
	int32 GetCurrentXP() const { return CurrentXP; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Experience")
	int32 GetCurrentLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintPure, Category = "Sandbox|Experience")
	int32 GetRequiredXP() const { return RequiredXP; }

	/** Retorna o limite de XP configurado para um nível específico */
	UFUNCTION(BlueprintPure, Category = "Sandbox|Experience")
	int32 GetRequiredXPForLevel(int32 Level) const;

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Experience")
	void SetRequiredXPDataTable(UDataTable* InDataTable);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Experience")
	void SetBaseRequiredXP(int32 InBaseXP);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Experience")
	void SetXPExponent(float InExponent);

	FSBExperienceChangedSignature OnExperienceChanged;

	FSBLevelUpSignature OnLevelUp;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentXP, VisibleAnywhere, BlueprintReadOnly, Category = "Sandbox|Experience")
	int32 CurrentXP;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentLevel, VisibleAnywhere, BlueprintReadOnly, Category = "Sandbox|Experience")
	int32 CurrentLevel;

	UPROPERTY(ReplicatedUsing = OnRep_RequiredXP, VisibleAnywhere, BlueprintReadOnly, Category = "Sandbox|Experience")
	int32 RequiredXP;

	/** Opcional: Tabela de dados contendo as linhas do tipo FRequiredXPRow para limites manuais */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Experience")
	UDataTable* RequiredXPDataTable;

	/** Valor de XP base usado se não houver DataTable ou se o nível exceder as linhas da tabela */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Experience", meta = (ClampMin = "10"))
	int32 BaseRequiredXP;

	/** Expoente de progressão para a curva exponencial: BaseRequiredXP * (Level ^ XPExponent) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox|Experience", meta = (ClampMin = "1.0"))
	float XPExponent;

	UFUNCTION()
	void OnRep_CurrentXP(int32 OldXP);

	UFUNCTION()
	void OnRep_CurrentLevel(int32 OldLevel);

	UFUNCTION()
	void OnRep_RequiredXP(int32 OldRequiredXP);

private:
	void UpdateRequiredXP();
};
