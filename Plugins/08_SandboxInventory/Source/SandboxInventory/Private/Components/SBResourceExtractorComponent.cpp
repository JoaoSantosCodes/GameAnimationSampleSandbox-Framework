#include "Components/SBResourceExtractorComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBResourceExtractorComponent::USBResourceExtractorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBResourceExtractorComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SimulateExtractorTick(0.0f);
}

void USBResourceExtractorComponent::SetupExtractor(ESBExtractorType InType, FName InItemId, ESBFluidType InFluidType, ESBResourceDepositPurity InPurity, float InBaseRate, float InPowerRequirement)
{
	ExtractorData.ExtractorType = InType;
	ExtractorData.ExtractedItemId = InItemId;
	ExtractorData.ExtractedFluidType = InFluidType;
	ExtractorData.DepositPurity = InPurity;
	ExtractorData.BaseExtractionRate = InBaseRate;
	ExtractorData.PowerConsumption = InPowerRequirement;
	ExtractorData.CurrentProgressAlpha = 0.0f;
	SimulateExtractorTick(0.0f);
}

void USBResourceExtractorComponent::SetOverclockMultiplier(float InMultiplier)
{
	ExtractorData.OverclockMultiplier = FMath::Clamp(InMultiplier, 0.1f, 5.0f);
}

void USBResourceExtractorComponent::SetPowerSupplied(bool bSupplied)
{
	ExtractorData.bHasPower = bSupplied;
	SimulateExtractorTick(0.0f);
}

void USBResourceExtractorComponent::SetDepositDepleted(bool bDepleted)
{
	ExtractorData.bIsDepositDepleted = bDepleted;
	SimulateExtractorTick(0.0f);
}

int32 USBResourceExtractorComponent::WithdrawOutputItem(FName ItemId, int32 Quantity)
{
	if (ItemId.IsNone() || Quantity <= 0)
	{
		return 0;
	}

	int32 Current = OutputItemBuffer.FindRef(ItemId);
	int32 Extracted = FMath::Min(Current, Quantity);
	OutputItemBuffer.FindOrAdd(ItemId) -= Extracted;

	SimulateExtractorTick(0.0f);
	return Extracted;
}

float USBResourceExtractorComponent::WithdrawOutputFluid(ESBFluidType FluidType, float Volume)
{
	if (FluidType == ESBFluidType::None || Volume <= 0.0f)
	{
		return 0.0f;
	}

	float Current = OutputFluidBuffer.FindRef(FluidType);
	float Extracted = FMath::Min(Current, Volume);
	OutputFluidBuffer.FindOrAdd(FluidType) -= Extracted;

	SimulateExtractorTick(0.0f);
	return Extracted;
}

int32 USBResourceExtractorComponent::GetOutputItemCount(FName ItemId) const
{
	return OutputItemBuffer.FindRef(ItemId);
}

float USBResourceExtractorComponent::GetOutputFluidVolume(ESBFluidType FluidType) const
{
	return OutputFluidBuffer.FindRef(FluidType);
}

float USBResourceExtractorComponent::GetPurityMultiplier() const
{
	switch (ExtractorData.DepositPurity)
	{
	case ESBResourceDepositPurity::Impure:
		return 0.5f;
	case ESBResourceDepositPurity::Pure:
		return 2.0f;
	case ESBResourceDepositPurity::Normal:
	default:
		return 1.0f;
	}
}

void USBResourceExtractorComponent::SimulateExtractorTick(float DeltaTime)
{
	// Dissipate heat
	if (DeltaTime > 0.0f)
	{
		ExtractorData.CurrentTemperature = FMath::Max(25.0f, ExtractorData.CurrentTemperature - Settings.HeatDissipationRate * DeltaTime);
	}

	// 1. Idle check (no resource configured)
	if (ExtractorData.ExtractedItemId.IsNone() && ExtractorData.ExtractedFluidType == ESBFluidType::None)
	{
		SyncExtractorState(ESBExtractorState::Idle);
		return;
	}

	// 2. Power check
	if (!ExtractorData.bHasPower)
	{
		SyncExtractorState(ESBExtractorState::NoPower);
		return;
	}

	// 3. Depletion check
	if (ExtractorData.bIsDepositDepleted)
	{
		SyncExtractorState(ESBExtractorState::Depleted);
		return;
	}

	// 4. Overheat check
	if (ExtractorData.CurrentTemperature >= ExtractorData.MaxSafeTemperature)
	{
		SyncExtractorState(ESBExtractorState::Overheated);
		return;
	}

	// 5. Output buffer check
	if (!ExtractorData.ExtractedItemId.IsNone())
	{
		int32 CurrentCount = OutputItemBuffer.FindRef(ExtractorData.ExtractedItemId);
		if (CurrentCount >= Settings.OutputItemBufferCapacity)
		{
			SyncExtractorState(ESBExtractorState::OutputBlocked);
			return;
		}
	}
	if (ExtractorData.ExtractedFluidType != ESBFluidType::None)
	{
		float CurrentVolume = OutputFluidBuffer.FindRef(ExtractorData.ExtractedFluidType);
		if (CurrentVolume >= Settings.OutputFluidBufferCapacity)
		{
			SyncExtractorState(ESBExtractorState::OutputBlocked);
			return;
		}
	}

	// 6. Extracting!
	SyncExtractorState(ESBExtractorState::Extracting);

	if (DeltaTime > 0.0f)
	{
		float EffectiveRate = ExtractorData.BaseExtractionRate * GetPurityMultiplier() * ExtractorData.OverclockMultiplier;
		ExtractorData.CurrentProgressAlpha += EffectiveRate * DeltaTime;
		ExtractorData.CurrentTemperature += ExtractorData.HeatGenerationRate * DeltaTime * ExtractorData.OverclockMultiplier;
		OnExtractorTemperatureChanged.Broadcast(ExtractorData.CurrentTemperature, ExtractorData.MaxSafeTemperature);

		while (ExtractorData.CurrentProgressAlpha >= 1.0f)
		{
			ExtractorData.CurrentProgressAlpha -= 1.0f;

			if (!ExtractorData.ExtractedItemId.IsNone())
			{
				int32 CurrentCount = OutputItemBuffer.FindRef(ExtractorData.ExtractedItemId);
				if (CurrentCount < Settings.OutputItemBufferCapacity)
				{
					OutputItemBuffer.FindOrAdd(ExtractorData.ExtractedItemId)++;
					ExtractorData.TotalExtractedItems++;
					OnExtractorItemHarvested.Broadcast(ExtractorData.ExtractedItemId, 1);
				}
				else
				{
					SyncExtractorState(ESBExtractorState::OutputBlocked);
					break;
				}
			}

			if (ExtractorData.ExtractedFluidType != ESBFluidType::None)
			{
				float CurrentVolume = OutputFluidBuffer.FindRef(ExtractorData.ExtractedFluidType);
				float VolumeToAdd = 10.0f; // 10 Litros por ciclo
				if (CurrentVolume + VolumeToAdd <= Settings.OutputFluidBufferCapacity)
				{
					OutputFluidBuffer.FindOrAdd(ExtractorData.ExtractedFluidType) += VolumeToAdd;
					ExtractorData.TotalExtractedFluids += VolumeToAdd;
					OnExtractorFluidHarvested.Broadcast(ExtractorData.ExtractedFluidType, VolumeToAdd);
				}
				else
				{
					SyncExtractorState(ESBExtractorState::OutputBlocked);
					break;
				}
			}
		}
	}
}

void USBResourceExtractorComponent::SyncExtractorState(ESBExtractorState NewState)
{
	if (ExtractorData.ExtractorState != NewState)
	{
		ExtractorData.ExtractorState = NewState;
		OnExtractorStateChanged.Broadcast(NewState);
		SyncTags();
	}
}

void USBResourceExtractorComponent::SyncTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	CachedStateComp->RemoveTag(Tags.State_Extractor_Drilling);
	CachedStateComp->RemoveTag(Tags.State_Extractor_Idle);
	CachedStateComp->RemoveTag(Tags.State_Extractor_Depleted);
	CachedStateComp->RemoveTag(Tags.State_Extractor_NoPower);
	CachedStateComp->RemoveTag(Tags.State_Extractor_OutputBlocked);
	CachedStateComp->RemoveTag(Tags.State_Extractor_Overheated);

	if (ExtractorData.ExtractorState == ESBExtractorState::Extracting)
	{
		CachedStateComp->AddTag(Tags.State_Extractor_Drilling);
	}
	else if (ExtractorData.ExtractorState == ESBExtractorState::Idle)
	{
		CachedStateComp->AddTag(Tags.State_Extractor_Idle);
	}
	else if (ExtractorData.ExtractorState == ESBExtractorState::Depleted)
	{
		CachedStateComp->AddTag(Tags.State_Extractor_Depleted);
	}
	else if (ExtractorData.ExtractorState == ESBExtractorState::NoPower)
	{
		CachedStateComp->AddTag(Tags.State_Extractor_NoPower);
	}
	else if (ExtractorData.ExtractorState == ESBExtractorState::OutputBlocked)
	{
		CachedStateComp->AddTag(Tags.State_Extractor_OutputBlocked);
	}
	else if (ExtractorData.ExtractorState == ESBExtractorState::Overheated)
	{
		CachedStateComp->AddTag(Tags.State_Extractor_Overheated);
	}
}
