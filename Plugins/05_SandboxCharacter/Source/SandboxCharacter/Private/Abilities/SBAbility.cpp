#include "Abilities/SBAbility.h"
#include "Components/SBStateComponent.h"
#include "Utilities/SBLogCategories.h"

USBAbility::USBAbility()
{
}

void USBAbility::Enter_Implementation(const FSBBehaviorContext& Context)
{
	Super::Enter_Implementation(Context);
	UE_LOG(LogSandboxCharacter, Log, TEXT("Activated Ability: %s"), *GetName());

	if (USBStateComponent* StateComp = Cast<USBStateComponent>(CachedStateComponent))
	{
		for (const FGameplayTag& Tag : AbilityTags)
		{
			StateComp->AddTag(Tag);
		}
	}
}

void USBAbility::Exit_Implementation(const FSBBehaviorContext& Context)
{
	Super::Exit_Implementation(Context);
	UE_LOG(LogSandboxCharacter, Log, TEXT("Ended Ability: %s"), *GetName());

	if (USBStateComponent* StateComp = Cast<USBStateComponent>(CachedStateComponent))
	{
		for (const FGameplayTag& Tag : AbilityTags)
		{
			StateComp->RemoveTag(Tag);
		}
	}
}
