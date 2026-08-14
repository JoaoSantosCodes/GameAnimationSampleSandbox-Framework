#include "SandboxInteraction.h"
#include "GameplayTagsManager.h"

#define LOCTEXT_NAMESPACE "FSandboxInteractionModule"

void FSandboxInteractionModule::StartupModule()
{
	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	TagsManager.AddNativeGameplayTag(TEXT("Event.Interaction.Available"), TEXT("Fired when interaction focus is available"));
	TagsManager.AddNativeGameplayTag(TEXT("Event.Interaction.Cleared"), TEXT("Fired when interaction focus is cleared"));
	TagsManager.AddNativeGameplayTag(TEXT("Event.Interaction.Started"), TEXT("Fired when hold interaction starts"));
	TagsManager.AddNativeGameplayTag(TEXT("Event.Interaction.Progress"), TEXT("Fired periodically during hold interaction"));
	TagsManager.AddNativeGameplayTag(TEXT("Event.Interaction.Completed"), TEXT("Fired when hold interaction completes"));
}

void FSandboxInteractionModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSandboxInteractionModule, SandboxInteraction)
