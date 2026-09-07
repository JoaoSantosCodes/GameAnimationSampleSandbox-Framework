// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBSandboxFeatureSubsystem.h"
#include "Subsystems/SBEventSubsystem.h"
#include "SBGameplayTags.h"
#include "Engine/GameInstance.h"

USBSandboxFeatureSubsystem::USBSandboxFeatureSubsystem()
	: Super()
{
}

bool USBSandboxFeatureSubsystem::IsFeatureEnabled(FGameplayTag FeatureTag) const
{
	if (!FeatureTag.IsValid())
	{
		return false;
	}
	return ActiveFeatures.HasTagExact(FeatureTag);
}

void USBSandboxFeatureSubsystem::EnableFeature(FGameplayTag FeatureTag)
{
	if (!FeatureTag.IsValid())
	{
		return;
	}

	if (!ActiveFeatures.HasTagExact(FeatureTag))
	{
		ActiveFeatures.AddTag(FeatureTag);
		
		OnFeatureToggled.Broadcast(FeatureTag, true);

		USBEventSubsystem* EventSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBEventSubsystem>() : nullptr;
		if (EventSubsystem)
		{
			USBSandboxFeaturePayload* Payload = NewObject<USBSandboxFeaturePayload>(this);
			Payload->FeatureTag = FeatureTag;
			Payload->bEnabled = true;

			FGameplayTag EventTag = FSBGameplayTags::Get().Event_Feature_Toggled;
			if (EventTag.IsValid())
			{
				EventSubsystem->PublishEvent(EventTag, Payload);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Sandbox Feature Enabled: %s"), *FeatureTag.ToString());
	}
}

void USBSandboxFeatureSubsystem::DisableFeature(FGameplayTag FeatureTag)
{
	if (!FeatureTag.IsValid())
	{
		return;
	}

	if (ActiveFeatures.HasTagExact(FeatureTag))
	{
		ActiveFeatures.RemoveTag(FeatureTag);
		
		OnFeatureToggled.Broadcast(FeatureTag, false);

		USBEventSubsystem* EventSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBEventSubsystem>() : nullptr;
		if (EventSubsystem)
		{
			USBSandboxFeaturePayload* Payload = NewObject<USBSandboxFeaturePayload>(this);
			Payload->FeatureTag = FeatureTag;
			Payload->bEnabled = false;

			FGameplayTag EventTag = FSBGameplayTags::Get().Event_Feature_Toggled;
			if (EventTag.IsValid())
			{
				EventSubsystem->PublishEvent(EventTag, Payload);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Sandbox Feature Disabled: %s"), *FeatureTag.ToString());
	}
}
