#include "Components/SBMetaBolicNutritionComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBMetaBolicNutritionComponent::USBMetaBolicNutritionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBMetaBolicNutritionComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	SyncTags();
}

void USBMetaBolicNutritionComponent::SetupMetabolicNutrition(float InitialCalories, float InitialHydration)
{
	NutritionData.Calories = FMath::Clamp(InitialCalories, 0.0f, NutritionData.MaxCalories);
	NutritionData.Hydration = FMath::Clamp(InitialHydration, 0.0f, NutritionData.MaxHydration);
	NutritionData.Nutrients.VitaminA = 100.0f;
	NutritionData.Nutrients.VitaminB = 100.0f;
	NutritionData.Nutrients.VitaminC = 100.0f;
	NutritionData.Nutrients.VitaminD = 100.0f;
	NutritionData.Nutrients.Electrolytes = 100.0f;
	NutritionData.ActivityState = ESBMetabolicActivityState::Resting;
	NutritionData.HungerLevel = (NutritionData.Calories >= 1500.0f) ? ESBHungerLevel::Satiated : ESBHungerLevel::Normal;
	NutritionData.HydrationLevel = (NutritionData.Hydration >= 60.0f) ? ESBHydrationLevel::Hydrated : ESBHydrationLevel::Thirsty;
	SyncTags();
}

void USBMetaBolicNutritionComponent::ConsumeFoodOrDrink(const FSBConsumableNutritionItem& Consumable)
{
	NutritionData.Calories = FMath::Clamp(NutritionData.Calories + Consumable.CalorieYield, 0.0f, NutritionData.MaxCalories);
	NutritionData.Hydration = FMath::Clamp(NutritionData.Hydration + Consumable.HydrationYield, 0.0f, NutritionData.MaxHydration);

	NutritionData.Nutrients.VitaminA = FMath::Clamp(NutritionData.Nutrients.VitaminA + Consumable.NutrientYield.VitaminA, 0.0f, 100.0f);
	NutritionData.Nutrients.VitaminB = FMath::Clamp(NutritionData.Nutrients.VitaminB + Consumable.NutrientYield.VitaminB, 0.0f, 100.0f);
	NutritionData.Nutrients.VitaminC = FMath::Clamp(NutritionData.Nutrients.VitaminC + Consumable.NutrientYield.VitaminC, 0.0f, 100.0f);
	NutritionData.Nutrients.VitaminD = FMath::Clamp(NutritionData.Nutrients.VitaminD + Consumable.NutrientYield.VitaminD, 0.0f, 100.0f);
	NutritionData.Nutrients.Electrolytes = FMath::Clamp(NutritionData.Nutrients.Electrolytes + Consumable.NutrientYield.Electrolytes, 0.0f, 100.0f);

	SimulateMetabolicTick(0.0f);
}

void USBMetaBolicNutritionComponent::SetActivityState(ESBMetabolicActivityState NewActivity)
{
	NutritionData.ActivityState = NewActivity;
}

float USBMetaBolicNutritionComponent::GetActivityMultiplier() const
{
	switch (NutritionData.ActivityState)
	{
	case ESBMetabolicActivityState::Resting:
		return 1.0f;
	case ESBMetabolicActivityState::Walking:
		return 1.5f;
	case ESBMetabolicActivityState::Sprinting:
		return 3.0f;
	case ESBMetabolicActivityState::Combat:
		return 4.0f;
	case ESBMetabolicActivityState::Shivering:
		return 2.5f;
	default:
		return 1.0f;
	}
}

bool USBMetaBolicNutritionComponent::HasDeficiency(FName NutrientName) const
{
	if (NutrientName == FName("VitaminC"))
	{
		return NutritionData.Nutrients.VitaminC < 15.0f;
	}
	if (NutrientName == FName("VitaminA"))
	{
		return NutritionData.Nutrients.VitaminA < 15.0f;
	}
	if (NutrientName == FName("Electrolytes"))
	{
		return NutritionData.Nutrients.Electrolytes < 15.0f;
	}
	return false;
}

void USBMetaBolicNutritionComponent::SimulateMetabolicTick(float DeltaTime, float AmbientHeatModifier)
{
	float ActivityMult = GetActivityMultiplier();

	if (DeltaTime > 0.0f)
	{
		// Burn calories
		float CalorieBurn = NutritionData.BaseBMR * ActivityMult * DeltaTime;
		NutritionData.Calories = FMath::Max(0.0f, NutritionData.Calories - CalorieBurn);

		// Burn hydration (affected by activity and ambient heat)
		float HydrationLoss = NutritionData.HydrationLossRate * ActivityMult * FMath::Max(0.5f, AmbientHeatModifier) * DeltaTime;
		NutritionData.Hydration = FMath::Max(0.0f, NutritionData.Hydration - HydrationLoss);

		// Micronutrients decay
		float NutrientDecay = 0.02f * DeltaTime;
		NutritionData.Nutrients.VitaminA = FMath::Max(0.0f, NutritionData.Nutrients.VitaminA - NutrientDecay);
		NutritionData.Nutrients.VitaminB = FMath::Max(0.0f, NutritionData.Nutrients.VitaminB - NutrientDecay);
		NutritionData.Nutrients.VitaminC = FMath::Max(0.0f, NutritionData.Nutrients.VitaminC - NutrientDecay);
		NutritionData.Nutrients.VitaminD = FMath::Max(0.0f, NutritionData.Nutrients.VitaminD - NutrientDecay);
		NutritionData.Nutrients.Electrolytes = FMath::Max(0.0f, NutritionData.Nutrients.Electrolytes - (NutrientDecay * ActivityMult));
	}

	// Update Hunger
	ESBHungerLevel OldHunger = NutritionData.HungerLevel;
	if (NutritionData.Calories <= 0.0f)
	{
		NutritionData.HungerLevel = ESBHungerLevel::Starving;
	}
	else if (NutritionData.Calories < 600.0f)
	{
		NutritionData.HungerLevel = ESBHungerLevel::Hungry;
	}
	else if (NutritionData.Calories < 1500.0f)
	{
		NutritionData.HungerLevel = ESBHungerLevel::Normal;
	}
	else
	{
		NutritionData.HungerLevel = ESBHungerLevel::Satiated;
	}

	if (OldHunger != NutritionData.HungerLevel)
	{
		OnHungerLevelChanged.Broadcast(OldHunger, NutritionData.HungerLevel);
		if (NutritionData.HungerLevel == ESBHungerLevel::Starving)
		{
			OnStarvationTriggered.Broadcast();
		}
	}

	// Update Hydration
	ESBHydrationLevel OldHydration = NutritionData.HydrationLevel;
	if (NutritionData.Hydration <= 0.0f)
	{
		NutritionData.HydrationLevel = ESBHydrationLevel::CriticalDehydration;
	}
	else if (NutritionData.Hydration < 30.0f)
	{
		NutritionData.HydrationLevel = ESBHydrationLevel::Dehydrated;
	}
	else if (NutritionData.Hydration < 60.0f)
	{
		NutritionData.HydrationLevel = ESBHydrationLevel::Thirsty;
	}
	else
	{
		NutritionData.HydrationLevel = ESBHydrationLevel::Hydrated;
	}

	if (OldHydration != NutritionData.HydrationLevel)
	{
		OnHydrationLevelChanged.Broadcast(OldHydration, NutritionData.HydrationLevel);
		if (NutritionData.HydrationLevel == ESBHydrationLevel::CriticalDehydration)
		{
			OnCriticalDehydrationTriggered.Broadcast();
		}
	}

	// Check deficiencies
	if (NutritionData.Nutrients.VitaminC < 15.0f)
	{
		OnNutrientDeficiencyTriggered.Broadcast(FName("VitaminC"));
	}
	if (NutritionData.Nutrients.VitaminA < 15.0f)
	{
		OnNutrientDeficiencyTriggered.Broadcast(FName("VitaminA"));
	}
	if (NutritionData.Nutrients.Electrolytes < 15.0f)
	{
		OnNutrientDeficiencyTriggered.Broadcast(FName("Electrolytes"));
	}

	SyncTags();
}

void USBMetaBolicNutritionComponent::SyncTags()
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

	CachedStateComp->RemoveTag(Tags.State_Metabolism_WellFed);
	CachedStateComp->RemoveTag(Tags.State_Metabolism_Hungry);
	CachedStateComp->RemoveTag(Tags.State_Metabolism_Starving);
	CachedStateComp->RemoveTag(Tags.State_Metabolism_Hydrated);
	CachedStateComp->RemoveTag(Tags.State_Metabolism_Thirsty);
	CachedStateComp->RemoveTag(Tags.State_Metabolism_Dehydrated);
	CachedStateComp->RemoveTag(Tags.State_Metabolism_Deficiency_VitaminC);
	CachedStateComp->RemoveTag(Tags.State_Metabolism_Deficiency_VitaminA);
	CachedStateComp->RemoveTag(Tags.State_Metabolism_Deficiency_Electrolytes);

	// Hunger tags
	if (NutritionData.HungerLevel == ESBHungerLevel::Satiated || NutritionData.HungerLevel == ESBHungerLevel::Normal)
	{
		CachedStateComp->AddTag(Tags.State_Metabolism_WellFed);
	}
	else if (NutritionData.HungerLevel == ESBHungerLevel::Hungry)
	{
		CachedStateComp->AddTag(Tags.State_Metabolism_Hungry);
	}
	else if (NutritionData.HungerLevel == ESBHungerLevel::Starving)
	{
		CachedStateComp->AddTag(Tags.State_Metabolism_Starving);
	}

	// Hydration tags
	if (NutritionData.HydrationLevel == ESBHydrationLevel::Hydrated)
	{
		CachedStateComp->AddTag(Tags.State_Metabolism_Hydrated);
	}
	else if (NutritionData.HydrationLevel == ESBHydrationLevel::Thirsty)
	{
		CachedStateComp->AddTag(Tags.State_Metabolism_Thirsty);
	}
	else if (NutritionData.HydrationLevel == ESBHydrationLevel::Dehydrated || NutritionData.HydrationLevel == ESBHydrationLevel::CriticalDehydration)
	{
		CachedStateComp->AddTag(Tags.State_Metabolism_Dehydrated);
	}

	// Deficiency tags
	if (NutritionData.Nutrients.VitaminC < 15.0f)
	{
		CachedStateComp->AddTag(Tags.State_Metabolism_Deficiency_VitaminC);
	}
	if (NutritionData.Nutrients.VitaminA < 15.0f)
	{
		CachedStateComp->AddTag(Tags.State_Metabolism_Deficiency_VitaminA);
	}
	if (NutritionData.Nutrients.Electrolytes < 15.0f)
	{
		CachedStateComp->AddTag(Tags.State_Metabolism_Deficiency_Electrolytes);
	}
}
