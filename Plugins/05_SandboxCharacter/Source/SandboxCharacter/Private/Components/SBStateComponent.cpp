#include "Components/SBStateComponent.h"
#include "Utilities/SBLogCategories.h"
#include "Net/UnrealNetwork.h"

USBStateComponent::USBStateComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true); // Habilita a replicação nativa do componente
}

void USBStateComponent::AddTag(FGameplayTag StateTag)
{
	if (!StateTag.IsValid()) return;

	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();
	const bool bIsLocallyControlled = GIsAutomationTesting || (Owner && Cast<APawn>(Owner) && Cast<APawn>(Owner)->IsLocallyControlled());

	if (bIsServer)
	{
		if (!ActiveStateTags.HasTagExact(StateTag))
		{
			ActiveStateTags.AddTag(StateTag);
			OnStateChanged.Broadcast(StateTag, true);
			UE_LOG(LogSandboxCharacter, Log, TEXT("Server added state tag: %s"), *StateTag.ToString());
		}
	}
	else if (bIsLocallyControlled)
	{
		if (!PredictedStateTags.HasTagExact(StateTag))
		{
			PredictedStateTags.AddTag(StateTag);
			OnStateChanged.Broadcast(StateTag, true);
			UE_LOG(LogSandboxCharacter, Log, TEXT("Client predicted state tag: %s"), *StateTag.ToString());
		}
	}
}

void USBStateComponent::RemoveTag(FGameplayTag StateTag)
{
	if (!StateTag.IsValid()) return;

	AActor* Owner = GetOwner();
	const bool bIsServer = Owner && Owner->HasAuthority();
	const bool bIsLocallyControlled = GIsAutomationTesting || (Owner && Cast<APawn>(Owner) && Cast<APawn>(Owner)->IsLocallyControlled());

	if (bIsServer)
	{
		if (ActiveStateTags.HasTagExact(StateTag))
		{
			ActiveStateTags.RemoveTag(StateTag);
			OnStateChanged.Broadcast(StateTag, false);
			UE_LOG(LogSandboxCharacter, Log, TEXT("Server removed state tag: %s"), *StateTag.ToString());
		}
	}
	else if (bIsLocallyControlled)
	{
		if (PredictedStateTags.HasTagExact(StateTag))
		{
			PredictedStateTags.RemoveTag(StateTag);
			OnStateChanged.Broadcast(StateTag, false);
			UE_LOG(LogSandboxCharacter, Log, TEXT("Client cleared predicted state tag: %s"), *StateTag.ToString());
		}
	}
}

bool USBStateComponent::HasTag(FGameplayTag StateTag) const
{
	return ActiveStateTags.HasTagExact(StateTag) || PredictedStateTags.HasTagExact(StateTag);
}

bool USBStateComponent::HasAny(FGameplayTagContainer TagsContainer) const
{
	return ActiveStateTags.HasAny(TagsContainer) || PredictedStateTags.HasAny(TagsContainer);
}

bool USBStateComponent::HasAll(FGameplayTagContainer TagsContainer) const
{
	FGameplayTagContainer Combined = ActiveStateTags;
	Combined.AppendTags(PredictedStateTags);
	return Combined.HasAll(TagsContainer);
}

void USBStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USBStateComponent, ActiveStateTags);
}

void USBStateComponent::OnRep_ActiveStateTags(const FGameplayTagContainer& OldTags)
{
	// Broadcast para adições na visão do cliente e limpeza de tags preditas locais correspondentes
	for (auto It = ActiveStateTags.CreateConstIterator(); It; ++It)
	{
		FGameplayTag Tag = *It;
		if (PredictedStateTags.HasTagExact(Tag))
		{
			PredictedStateTags.RemoveTag(Tag);
		}
		else if (!OldTags.HasTagExact(Tag))
		{
			OnStateChanged.Broadcast(Tag, true);
		}
	}

	// Broadcast para remoções na visão do cliente e garantia de limpeza local
	for (auto It = OldTags.CreateConstIterator(); It; ++It)
	{
		FGameplayTag Tag = *It;
		if (!ActiveStateTags.HasTagExact(Tag))
		{
			if (PredictedStateTags.HasTagExact(Tag))
			{
				PredictedStateTags.RemoveTag(Tag);
			}
			OnStateChanged.Broadcast(Tag, false);
		}
	}
}

void USBStateComponent::GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const
{
	FSBDebugLine Header;
	Header.Label = GetClass()->GetName();
	Header.bIsHeader = true;
	OutDebugLines.Add(Header);

	// Replicated state tags
	FSBDebugLine ActiveHeader;
	ActiveHeader.Label = FString::Printf(TEXT("Active State Tags (%d)"), ActiveStateTags.Num());
	ActiveHeader.bIsHeader = true;
	OutDebugLines.Add(ActiveHeader);

	for (auto It = ActiveStateTags.CreateConstIterator(); It; ++It)
	{
		FSBDebugLine Line;
		Line.Label = TEXT("  Tag");
		Line.Value = It->ToString();
		OutDebugLines.Add(Line);
	}

	// Predicted state tags (Client-only / Local)
	if (PredictedStateTags.Num() > 0)
	{
		FSBDebugLine PredHeader;
		PredHeader.Label = FString::Printf(TEXT("Predicted State Tags (%d)"), PredictedStateTags.Num());
		PredHeader.bIsHeader = true;
		OutDebugLines.Add(PredHeader);

		for (auto It = PredictedStateTags.CreateConstIterator(); It; ++It)
		{
			FSBDebugLine Line;
			Line.Label = TEXT("  Predicted");
			Line.Value = It->ToString();
			OutDebugLines.Add(Line);
		}
	}
}
