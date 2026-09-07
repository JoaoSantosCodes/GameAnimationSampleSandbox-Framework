// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBSandboxBackgroundSimSubsystem.h"
#include "Subsystems/SBSaveSubsystemConcrete.h"
#include "Misc/DateTime.h"

USBSandboxBackgroundSimSubsystem::USBSandboxBackgroundSimSubsystem()
{
}

void USBSandboxBackgroundSimSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	TimeSinceLastTick = 0.0f;
}

void USBSandboxBackgroundSimSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USBSandboxBackgroundSimSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsTickEnabled) return;

	TimeSinceLastTick += DeltaTime;
	if (TimeSinceLastTick >= BackgroundTickInterval)
	{
		ForceAdvanceTime(TimeSinceLastTick);
		TimeSinceLastTick = 0.0f;
	}
}

TStatId USBSandboxBackgroundSimSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USBSandboxBackgroundSimSubsystem, STATGROUP_Tickables);
}

void USBSandboxBackgroundSimSubsystem::RegisterSimulatedEntity(const FSBSimulatedEntityData& EntityData)
{
	if (!EntityData.EntityId.IsValid()) return;
	ActiveSimulations.Add(EntityData.EntityId, EntityData);
}

void USBSandboxBackgroundSimSubsystem::UnregisterSimulatedEntity(FGuid EntityId)
{
	ActiveSimulations.Remove(EntityId);
}

bool USBSandboxBackgroundSimSubsystem::GetSimulatedEntityData(FGuid EntityId, FSBSimulatedEntityData& OutData) const
{
	const FSBSimulatedEntityData* FoundData = ActiveSimulations.Find(EntityId);
	if (FoundData)
	{
		OutData = *FoundData;
		return true;
	}
	return false;
}

void USBSandboxBackgroundSimSubsystem::ClearAllSimulations()
{
	ActiveSimulations.Empty();
	LastSavedTimestamp = 0;
}

void USBSandboxBackgroundSimSubsystem::ForceAdvanceTime(float DeltaSeconds)
{
	if (DeltaSeconds <= 0.0f) return;

	for (auto& Pair : ActiveSimulations)
	{
		FSBSimulatedEntityData& Entity = Pair.Value;

		// Atualiza todos os estados numéricos de contagem regressiva (timers)
		for (auto& StatePair : Entity.NumericStates)
		{
			if (StatePair.Key.ToString().Contains(TEXT("Timer")))
			{
				StatePair.Value = FMath::Max(0.0f, StatePair.Value - DeltaSeconds);
			}
		}
	}
}

bool USBSandboxBackgroundSimSubsystem::SaveComponentData_Implementation(UObject* SavePayload)
{
	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (Payload)
	{
		LastSavedTimestamp = FDateTime::UtcNow().ToUnixTimestamp();
		Payload->SerializeObject(GetPathName(), this);
		return true;
	}
	return false;
}

bool USBSandboxBackgroundSimSubsystem::LoadComponentData_Implementation(UObject* SavePayload)
{
	USBSavePayload* Payload = Cast<USBSavePayload>(SavePayload);
	if (Payload)
	{
		Payload->DeserializeObject(GetPathName(), this);

		// Executa Catch-up temporal baseado no tempo transcorrido no mundo real
		int64 CurrentTimestamp = FDateTime::UtcNow().ToUnixTimestamp();
		if (LastSavedTimestamp > 0 && CurrentTimestamp > LastSavedTimestamp)
		{
			float ElapsedTime = static_cast<float>(CurrentTimestamp - LastSavedTimestamp);
			ForceAdvanceTime(ElapsedTime);
		}
		return true;
	}
	return false;
}
