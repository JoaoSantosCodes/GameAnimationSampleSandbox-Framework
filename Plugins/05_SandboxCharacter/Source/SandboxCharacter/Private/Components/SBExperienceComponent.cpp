// Copyright 2026 João Santos. All Rights Reserved.
// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SBExperienceComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

USBExperienceComponent::USBExperienceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	CurrentXP = 0;
	CurrentLevel = 1;
	RequiredXP = 100;
	BaseRequiredXP = 100;
	XPExponent = 1.5f;
	RequiredXPDataTable = nullptr;
}

void USBExperienceComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBExperienceComponent, CurrentXP);
	DOREPLIFETIME(USBExperienceComponent, CurrentLevel);
	DOREPLIFETIME(USBExperienceComponent, RequiredXP);
}

void USBExperienceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UpdateRequiredXP();
	}
}

void USBExperienceComponent::AddExperience(int32 Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (Amount <= 0)
	{
		return;
	}

	int32 RemainingXP = Amount;
	int32 OriginalLevel = CurrentLevel;

	while (CurrentXP + RemainingXP >= RequiredXP)
	{
		int32 XPToLevelUp = RequiredXP - CurrentXP;
		RemainingXP -= XPToLevelUp;
		CurrentXP = 0;
		CurrentLevel++;

		OnLevelUp.Broadcast(CurrentLevel);
		UpdateRequiredXP();
	}

	CurrentXP += RemainingXP;
	OnExperienceChanged.Broadcast(CurrentXP, Amount, RequiredXP);
}

int32 USBExperienceComponent::GetRequiredXPForLevel(int32 Level) const
{
	if (Level <= 0)
	{
		return 0;
	}

	if (RequiredXPDataTable)
	{
		FName RowName = FName(*FString::FromInt(Level));
		FRequiredXPRow* Row = RequiredXPDataTable->FindRow<FRequiredXPRow>(RowName, TEXT("RequiredXPDataTable"));
		if (Row)
		{
			return Row->RequiredXP;
		}
	}

	// Fórmula padrão se não houver DataTable ou se o nível não for mapeado
	float PowVal = FMath::Pow(static_cast<float>(Level), XPExponent);
	return FMath::RoundToInt(static_cast<float>(BaseRequiredXP) * PowVal);
}

void USBExperienceComponent::UpdateRequiredXP()
{
	RequiredXP = GetRequiredXPForLevel(CurrentLevel);
}

void USBExperienceComponent::SetRequiredXPDataTable(UDataTable* InDataTable)
{
	RequiredXPDataTable = InDataTable;
	UpdateRequiredXP();
}

void USBExperienceComponent::SetBaseRequiredXP(int32 InBaseXP)
{
	BaseRequiredXP = FMath::Max(10, InBaseXP);
	UpdateRequiredXP();
}

void USBExperienceComponent::SetXPExponent(float InExponent)
{
	XPExponent = FMath::Max(1.0f, InExponent);
	UpdateRequiredXP();
}

void USBExperienceComponent::OnRep_CurrentXP(int32 OldXP)
{
	OnExperienceChanged.Broadcast(CurrentXP, CurrentXP - OldXP, RequiredXP);
}

void USBExperienceComponent::OnRep_CurrentLevel(int32 OldLevel)
{
	OnLevelUp.Broadcast(CurrentLevel);
}

void USBExperienceComponent::OnRep_RequiredXP(int32 OldRequiredXP)
{
	// Apenas para sincronizar localmente
}
