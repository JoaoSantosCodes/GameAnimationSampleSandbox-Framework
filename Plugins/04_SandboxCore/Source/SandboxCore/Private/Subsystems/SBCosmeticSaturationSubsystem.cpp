// Copyright 2026 João Santos. All Rights Reserved.
// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/SBCosmeticSaturationSubsystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

USBCosmeticSaturationSubsystem::USBCosmeticSaturationSubsystem()
{
}

void USBCosmeticSaturationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(CleanupTimerHandle, this, &USBCosmeticSaturationSubsystem::ResetObsoleteRecords, 10.0f, true);
	}
}

void USBCosmeticSaturationSubsystem::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CleanupTimerHandle);
	}

	SoundRecords.Empty();
	EffectRecords.Empty();

	Super::Deinitialize();
}

bool USBCosmeticSaturationSubsystem::AllowSound(USoundBase* Sound, const FVector& Location, float MinInterval)
{
	if (!Sound || !GetWorld())
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	FString Key = GetSpatialKey(Sound, Location);

	FSBCosmeticLimitRecord& Record = SoundRecords.FindOrAdd(Key);
	if (Record.FramePlayCount > 0 && (CurrentTime - Record.LastPlayTime) < MinInterval)
	{
		return false;
	}

	Record.LastPlayTime = CurrentTime;
	Record.FramePlayCount++;
	return true;
}

bool USBCosmeticSaturationSubsystem::AllowEffect(UObject* EffectAsset, const FVector& Location, float MinInterval)
{
	if (!EffectAsset || !GetWorld())
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	FString Key = GetSpatialKey(EffectAsset, Location);

	FSBCosmeticLimitRecord& Record = EffectRecords.FindOrAdd(Key);
	if (Record.FramePlayCount > 0 && (CurrentTime - Record.LastPlayTime) < MinInterval)
	{
		return false;
	}

	Record.LastPlayTime = CurrentTime;
	Record.FramePlayCount++;
	return true;
}

void USBCosmeticSaturationSubsystem::ResetObsoleteRecords()
{
	if (!GetWorld())
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ExpiryThreshold = 30.0f;

	// Limpa sons obsoletos
	for (auto It = SoundRecords.CreateIterator(); It; ++It)
	{
		if (CurrentTime - It.Value().LastPlayTime > ExpiryThreshold)
		{
			It.RemoveCurrent();
		}
	}

	// Limpa efeitos obsoletos
	for (auto It = EffectRecords.CreateIterator(); It; ++It)
	{
		if (CurrentTime - It.Value().LastPlayTime > ExpiryThreshold)
		{
			It.RemoveCurrent();
		}
	}
}

FString USBCosmeticSaturationSubsystem::GetSpatialKey(UObject* Asset, const FVector& Location) const
{
	// Grid espacial de 1 metro (100 unidades Unreal)
	int32 GridX = FMath::RoundToInt(Location.X / 100.0f);
	int32 GridY = FMath::RoundToInt(Location.Y / 100.0f);
	int32 GridZ = FMath::RoundToInt(Location.Z / 100.0f);

	FString AssetName = Asset ? Asset->GetName() : TEXT("None");
	return FString::Printf(TEXT("%s_%d_%d_%d"), *AssetName, GridX, GridY, GridZ);
}
