#include "Components/SBIndustrialProcessorComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBIndustrialProcessorComponent::USBIndustrialProcessorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBIndustrialProcessorComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SimulateProcessorTick(0.0f);
}

void USBIndustrialProcessorComponent::SetupProcessor(ESBProcessorType InType, const FSBIndustrialRecipe& InRecipe)
{
	ProcessorData.ProcessorType = InType;
	ProcessorData.ActiveRecipe = InRecipe;
	ProcessorData.CurrentProgressAlpha = 0.0f;
	SimulateProcessorTick(0.0f);
}

void USBIndustrialProcessorComponent::SetOverclockMultiplier(float InMultiplier)
{
	ProcessorData.OverclockMultiplier = FMath::Clamp(InMultiplier, 0.1f, 5.0f);
}

void USBIndustrialProcessorComponent::SetPowerSupplied(bool bSupplied)
{
	ProcessorData.bHasPower = bSupplied;
	SimulateProcessorTick(0.0f);
}

int32 USBIndustrialProcessorComponent::DepositInputItem(FName ItemId, int32 Quantity)
{
	if (ItemId.IsNone() || Quantity <= 0)
	{
		return 0;
	}

	int32 Current = InputItemBuffer.FindRef(ItemId);
	int32 Space = FMath::Max(0, Settings.InputItemBufferCapacity - Current);
	int32 Accepted = FMath::Min(Space, Quantity);
	InputItemBuffer.FindOrAdd(ItemId) += Accepted;

	SimulateProcessorTick(0.0f);
	return Accepted;
}

float USBIndustrialProcessorComponent::DepositInputFluid(ESBFluidType FluidType, float Volume)
{
	if (FluidType == ESBFluidType::None || Volume <= 0.0f)
	{
		return 0.0f;
	}

	float Current = InputFluidBuffer.FindRef(FluidType);
	float Space = FMath::Max(0.0f, Settings.FluidBufferCapacity - Current);
	float Accepted = FMath::Min(Space, Volume);
	InputFluidBuffer.FindOrAdd(FluidType) += Accepted;

	SimulateProcessorTick(0.0f);
	return Accepted;
}

int32 USBIndustrialProcessorComponent::WithdrawOutputItem(FName ItemId, int32 Quantity)
{
	if (ItemId.IsNone() || Quantity <= 0)
	{
		return 0;
	}

	int32 Current = OutputItemBuffer.FindRef(ItemId);
	int32 Extracted = FMath::Min(Current, Quantity);
	OutputItemBuffer.FindOrAdd(ItemId) -= Extracted;

	SimulateProcessorTick(0.0f);
	return Extracted;
}

float USBIndustrialProcessorComponent::WithdrawOutputFluid(ESBFluidType FluidType, float Volume)
{
	if (FluidType == ESBFluidType::None || Volume <= 0.0f)
	{
		return 0.0f;
	}

	float Current = OutputFluidBuffer.FindRef(FluidType);
	float Extracted = FMath::Min(Current, Volume);
	OutputFluidBuffer.FindOrAdd(FluidType) -= Extracted;

	SimulateProcessorTick(0.0f);
	return Extracted;
}

int32 USBIndustrialProcessorComponent::GetInputItemCount(FName ItemId) const
{
	return InputItemBuffer.FindRef(ItemId);
}

int32 USBIndustrialProcessorComponent::GetOutputItemCount(FName ItemId) const
{
	return OutputItemBuffer.FindRef(ItemId);
}

float USBIndustrialProcessorComponent::GetInputFluidVolume(ESBFluidType FluidType) const
{
	return InputFluidBuffer.FindRef(FluidType);
}

float USBIndustrialProcessorComponent::GetOutputFluidVolume(ESBFluidType FluidType) const
{
	return OutputFluidBuffer.FindRef(FluidType);
}

void USBIndustrialProcessorComponent::SimulateProcessorTick(float DeltaTime)
{
	// Dissipate heat
	if (DeltaTime > 0.0f)
	{
		ProcessorData.CurrentTemperature = FMath::Max(25.0f, ProcessorData.CurrentTemperature - Settings.HeatDissipationRate * DeltaTime);
	}

	// 1. Recipe
	if (ProcessorData.ActiveRecipe.RecipeId.IsNone())
	{
		SyncProcessorState(ESBProcessorState::Idle);
		return;
	}

	// 2. Power
	if (!ProcessorData.bHasPower)
	{
		SyncProcessorState(ESBProcessorState::NoPower);
		return;
	}

	// 3. Overheat
	if (ProcessorData.CurrentTemperature >= ProcessorData.MaxSafeTemperature)
	{
		SyncProcessorState(ESBProcessorState::Overheated);
		return;
	}

	// 4. Check Output Buffer Room
	for (const FSBIndustrialIngredient& OutItem : ProcessorData.ActiveRecipe.OutputItems)
	{
		int32 CurrentOut = OutputItemBuffer.FindRef(OutItem.ItemId);
		if (CurrentOut + OutItem.Quantity > Settings.OutputItemBufferCapacity)
		{
			SyncProcessorState(ESBProcessorState::OutputFull);
			return;
		}
	}
	for (const FSBIndustrialFluidIngredient& OutFluid : ProcessorData.ActiveRecipe.OutputFluids)
	{
		float CurrentOut = OutputFluidBuffer.FindRef(OutFluid.FluidType);
		if (CurrentOut + OutFluid.Volume > Settings.FluidBufferCapacity)
		{
			SyncProcessorState(ESBProcessorState::OutputFull);
			return;
		}
	}

	// 5. Check Ingredients
	for (const FSBIndustrialIngredient& InItem : ProcessorData.ActiveRecipe.InputItems)
	{
		if (InputItemBuffer.FindRef(InItem.ItemId) < InItem.Quantity)
		{
			SyncProcessorState(ESBProcessorState::MissingIngredients);
			return;
		}
	}
	for (const FSBIndustrialFluidIngredient& InFluid : ProcessorData.ActiveRecipe.InputFluids)
	{
		if (InputFluidBuffer.FindRef(InFluid.FluidType) < InFluid.Volume)
		{
			SyncProcessorState(ESBProcessorState::MissingIngredients);
			return;
		}
	}

	// 6. Process!
	SyncProcessorState(ESBProcessorState::Processing);

	if (DeltaTime > 0.0f)
	{
		float Rate = (ProcessorData.ActiveRecipe.CraftingTime > 0.0f) ? (1.0f / ProcessorData.ActiveRecipe.CraftingTime) * ProcessorData.OverclockMultiplier : 1.0f;
		ProcessorData.CurrentProgressAlpha += Rate * DeltaTime;
		ProcessorData.CurrentTemperature += ProcessorData.ActiveRecipe.HeatGeneration * DeltaTime * ProcessorData.OverclockMultiplier;
		OnProcessorTemperatureChanged.Broadcast(ProcessorData.CurrentTemperature, ProcessorData.MaxSafeTemperature);

		if (ProcessorData.CurrentProgressAlpha >= 1.0f)
		{
			// Deduct inputs
			for (const FSBIndustrialIngredient& InItem : ProcessorData.ActiveRecipe.InputItems)
			{
				InputItemBuffer.FindOrAdd(InItem.ItemId) -= InItem.Quantity;
			}
			for (const FSBIndustrialFluidIngredient& InFluid : ProcessorData.ActiveRecipe.InputFluids)
			{
				InputFluidBuffer.FindOrAdd(InFluid.FluidType) -= InFluid.Volume;
			}

			// Grant outputs
			for (const FSBIndustrialIngredient& OutItem : ProcessorData.ActiveRecipe.OutputItems)
			{
				OutputItemBuffer.FindOrAdd(OutItem.ItemId) += OutItem.Quantity;
			}
			for (const FSBIndustrialFluidIngredient& OutFluid : ProcessorData.ActiveRecipe.OutputFluids)
			{
				OutputFluidBuffer.FindOrAdd(OutFluid.FluidType) += OutFluid.Volume;
			}

			ProcessorData.CurrentProgressAlpha = 0.0f;
			ProcessorData.CompletedCyclesCount++;
			OnProcessorCycleCompleted.Broadcast(ProcessorData.ActiveRecipe.RecipeId, ProcessorData.CompletedCyclesCount);
		}
	}
}

void USBIndustrialProcessorComponent::SyncProcessorState(ESBProcessorState NewState)
{
	if (ProcessorData.ProcessorState != NewState)
	{
		ProcessorData.ProcessorState = NewState;
		OnProcessorStateChanged.Broadcast(NewState);
		SyncTags();
	}
}

void USBIndustrialProcessorComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Industrial_Processing);
	CachedStateComp->RemoveTag(Tags.State_Industrial_Idle);
	CachedStateComp->RemoveTag(Tags.State_Industrial_MissingIngredients);
	CachedStateComp->RemoveTag(Tags.State_Industrial_NoPower);
	CachedStateComp->RemoveTag(Tags.State_Industrial_OutputFull);
	CachedStateComp->RemoveTag(Tags.State_Industrial_Overheated);

	if (ProcessorData.ProcessorState == ESBProcessorState::Processing)
	{
		CachedStateComp->AddTag(Tags.State_Industrial_Processing);
	}
	else if (ProcessorData.ProcessorState == ESBProcessorState::Idle)
	{
		CachedStateComp->AddTag(Tags.State_Industrial_Idle);
	}
	else if (ProcessorData.ProcessorState == ESBProcessorState::MissingIngredients)
	{
		CachedStateComp->AddTag(Tags.State_Industrial_MissingIngredients);
	}
	else if (ProcessorData.ProcessorState == ESBProcessorState::NoPower)
	{
		CachedStateComp->AddTag(Tags.State_Industrial_NoPower);
	}
	else if (ProcessorData.ProcessorState == ESBProcessorState::OutputFull)
	{
		CachedStateComp->AddTag(Tags.State_Industrial_OutputFull);
	}
	else if (ProcessorData.ProcessorState == ESBProcessorState::Overheated)
	{
		CachedStateComp->AddTag(Tags.State_Industrial_Overheated);
	}
}
