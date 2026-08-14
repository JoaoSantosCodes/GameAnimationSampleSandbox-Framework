#include "Components/SBBehaviorStackComponent.h"
#include "Behaviors/SBGameplayBehaviorDefinition.h"
#include "Interfaces/SBCharacterInterface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Subsystems/SBSaveSubsystem.h"

USBBehaviorStackComponent::USBBehaviorStackComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBBehaviorStackComponent::OnShutdown_Implementation()
{
	FSBGameplayContext GameplayCtx;
	FSBFrameworkContext FrameworkCtx;
	FSBBehaviorContext Context = BuildBehaviorContext(0.f, GameplayCtx, FrameworkCtx);

	for (USBGameplayBehavior* Active : ActiveBehaviors)
	{
		if (Active)
		{
			Active->Exit(Context);
		}
	}
	ActiveBehaviors.Empty();
	AvailableBehaviors.Empty();
}

void USBBehaviorStackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FSBBehaviorStackMutationGuard Guard(this);

	FSBGameplayContext GameplayCtx;
	FSBFrameworkContext FrameworkCtx;
	FSBBehaviorContext Context = BuildBehaviorContext(DeltaTime, GameplayCtx, FrameworkCtx);

	for (USBGameplayBehavior* Active : ActiveBehaviors)
	{
		if (Active && !DeferredExits.Contains(Active))
		{
			Active->Update(DeltaTime, Context);
		}
	}
}

bool USBBehaviorStackComponent::RequestBehavior(FGameplayTag BehaviorTag)
{
	if (!BehaviorTag.IsValid()) return false;

	USBGameplayBehavior* TargetBehavior = FindAvailableBehaviorByTag(BehaviorTag);
	if (!TargetBehavior) return false;

	if (HasBehavior(BehaviorTag)) return true;

	FSBBehaviorStackMutationGuard Guard(this);

	FSBGameplayContext GameplayCtx;
	FSBFrameworkContext FrameworkCtx;
	FSBBehaviorContext Context = BuildBehaviorContext(0.f, GameplayCtx, FrameworkCtx);

	if (!TargetBehavior->CanEnter(Context))
	{
		return false;
	}

	// Ejeta behaviors conflitantes baseados no ExclusivityGroup
	if (TargetBehavior->GetDefinition() && TargetBehavior->GetDefinition()->ExclusivityGroup.IsValid())
	{
		FGameplayTag Group = TargetBehavior->GetDefinition()->ExclusivityGroup;
		for (int32 i = ActiveBehaviors.Num() - 1; i >= 0; --i)
		{
			USBGameplayBehavior* Active = ActiveBehaviors[i];
			if (Active && Active->GetDefinition() && Active->GetDefinition()->ExclusivityGroup == Group)
			{
				StopBehavior(Active->GetDefinition()->BehaviorTag, true, false);
			}
		}
	}

	if (StackMutationDepth > 1)
	{
		DeferredEntries.AddUnique(TargetBehavior);
		return true;
	}

	ActiveBehaviors.Add(TargetBehavior);
	TargetBehavior->Enter(Context);

	SortActiveStack();
	return true;
}

void USBBehaviorStackComponent::StopBehavior(FGameplayTag BehaviorTag, bool bSkipServerNotify, bool bSkipClientNotify)
{
	if (!BehaviorTag.IsValid()) return;

	USBGameplayBehavior* ActiveBehavior = FindActiveBehaviorByTag(BehaviorTag);
	if (ActiveBehavior && !DeferredExits.Contains(ActiveBehavior))
	{
		AActor* Owner = GetOwner();
		const bool bIsServer = Owner && Owner->HasAuthority();
		const bool bIsLocallyControlled = (Owner && Cast<APawn>(Owner) && Cast<APawn>(Owner)->IsLocallyControlled());

		if (!bIsServer && !bIsLocallyControlled)
		{
			return;
		}

		IncrementMutationDepth();

		// 1. Executa a saída lógica imediata para retirar modificadores e tags de estado no mesmo frame
		FSBGameplayContext GameplayCtx;
		FSBFrameworkContext FrameworkCtx;
		FSBBehaviorContext Context = BuildBehaviorContext(0.f, GameplayCtx, FrameworkCtx);
		ActiveBehavior->Exit(Context);

		// 2. Enfileira a remoção física da lista para o final do tick
		DeferredExits.AddUnique(ActiveBehavior);

		// 3. Dispara o gancho virtual para notificação de rede por domínio
		OnBehaviorEjected(BehaviorTag, bSkipServerNotify, bSkipClientNotify);

		DecrementMutationDepth();
	}
}

bool USBBehaviorStackComponent::HasBehavior(FGameplayTag BehaviorTag) const
{
	return FindActiveBehaviorByTag(BehaviorTag) != nullptr;
}

USBGameplayBehavior* USBBehaviorStackComponent::GetCurrentBehavior() const
{
	return ActiveBehaviors.Num() > 0 ? ActiveBehaviors[0] : nullptr;
}

USBGameplayBehavior* USBBehaviorStackComponent::FindAvailableBehaviorByTag(FGameplayTag Tag) const
{
	for (USBGameplayBehavior* Behavior : AvailableBehaviors)
	{
		if (Behavior && Behavior->GetDefinition() && Behavior->GetDefinition()->BehaviorTag == Tag)
		{
			return Behavior;
		}
	}
	return nullptr;
}

USBGameplayBehavior* USBBehaviorStackComponent::FindActiveBehaviorByTag(FGameplayTag Tag) const
{
	for (USBGameplayBehavior* Behavior : ActiveBehaviors)
	{
		if (Behavior && Behavior->GetDefinition() && Behavior->GetDefinition()->BehaviorTag == Tag)
		{
			return Behavior;
		}
	}
	return nullptr;
}

void USBBehaviorStackComponent::IncrementMutationDepth()
{
	StackMutationDepth++;
}

void USBBehaviorStackComponent::DecrementMutationDepth()
{
	StackMutationDepth--;
	if (StackMutationDepth == 0 && !bIsResolvingDeferred)
	{
		bIsResolvingDeferred = true;

		while (DeferredExits.Num() > 0 || DeferredEntries.Num() > 0)
		{
			ResolveDeferredExits();
			ResolveDeferredEntries();
		}

		bIsResolvingDeferred = false;
	}
}

void USBBehaviorStackComponent::ResolveDeferredExits()
{
	if (DeferredExits.Num() == 0) return;

	for (USBGameplayBehavior* Behavior : DeferredExits)
	{
		if (Behavior && ActiveBehaviors.Contains(Behavior))
		{
			ActiveBehaviors.Remove(Behavior);
		}
	}

	DeferredExits.Empty();
	SortActiveStack();
}

void USBBehaviorStackComponent::ResolveDeferredEntries()
{
	if (DeferredEntries.Num() == 0) return;

	TArray<TObjectPtr<USBGameplayBehavior>> TempEntries = DeferredEntries;
	DeferredEntries.Empty();

	for (USBGameplayBehavior* Behavior : TempEntries)
	{
		if (Behavior && Behavior->GetDefinition())
		{
			RequestBehavior(Behavior->GetDefinition()->BehaviorTag);
		}
	}
}

void USBBehaviorStackComponent::SortActiveStack()
{
	ActiveBehaviors.Sort([](const USBGameplayBehavior& A, const USBGameplayBehavior& B)
	{
		return A.GetStackPriority() > B.GetStackPriority();
	});
}

FSBBehaviorContext USBBehaviorStackComponent::BuildBehaviorContext(float DeltaTime, FSBGameplayContext& OutGameplayCtx, FSBFrameworkContext& OutFrameworkCtx) const
{
	OutGameplayCtx.DeltaSeconds = DeltaTime;

	AActor* Owner = GetOwner();
	if (Owner)
	{
		OutGameplayCtx.Pawn = Cast<APawn>(Owner);
		OutGameplayCtx.Character = Cast<ACharacter>(Owner);

		if (OutGameplayCtx.Pawn)
		{
			OutGameplayCtx.Controller = OutGameplayCtx.Pawn->GetController();
			OutGameplayCtx.PlayerState = OutGameplayCtx.Pawn->GetPlayerState();
		}
	}

	OutFrameworkCtx.World = GetWorld();
	if (OutFrameworkCtx.World)
	{
		OutFrameworkCtx.GameInstance = OutFrameworkCtx.World->GetGameInstance();
	}
	OutFrameworkCtx.EventSubsystem = nullptr;
	OutFrameworkCtx.AssetManager = nullptr;
	OutFrameworkCtx.SaveSubsystem = OutFrameworkCtx.GameInstance ? OutFrameworkCtx.GameInstance->GetSubsystem<USBSaveSubsystem>() : nullptr;

	FSBBehaviorContext Context;
	Context.GameplayContext = &OutGameplayCtx;
	Context.FrameworkContext = &OutFrameworkCtx;
	return Context;
}

void USBMockGameplayBehavior::Enter_Implementation(const FSBBehaviorContext& Context)
{
	bEntered = true;
	Super::Enter_Implementation(Context);
}

void USBMockGameplayBehavior::Exit_Implementation(const FSBBehaviorContext& Context)
{
	bExited = true;
	Super::Exit_Implementation(Context);
	if (StackToRequestOnExit && TagToRequestOnExit.IsValid())
	{
		StackToRequestOnExit->RequestBehavior(TagToRequestOnExit);
	}
}

void USBBehaviorStackComponent::GetDebugDescription_Implementation(TArray<FSBDebugLine>& OutDebugLines) const
{
	FSBDebugLine Header;
	Header.Label = GetClass()->GetName();
	Header.bIsHeader = true;
	OutDebugLines.Add(Header);

	FSBDebugLine StateLine;
	StateLine.Label = TEXT("Mutation Depth");
	StateLine.Value = FString::FromInt(StackMutationDepth);
	OutDebugLines.Add(StateLine);

	FSBDebugLine ResolvingLine;
	ResolvingLine.Label = TEXT("Is Resolving Deferred");
	ResolvingLine.Value = bIsResolvingDeferred ? TEXT("Yes") : TEXT("No");
	OutDebugLines.Add(ResolvingLine);

	// Active Stack
	FSBDebugLine StackHeader;
	StackHeader.Label = FString::Printf(TEXT("Active Behaviors (%d)"), ActiveBehaviors.Num());
	StackHeader.bIsHeader = true;
	OutDebugLines.Add(StackHeader);

	for (int32 i = 0; i < ActiveBehaviors.Num(); ++i)
	{
		USBGameplayBehavior* Behavior = ActiveBehaviors[i];
		if (Behavior)
		{
			FSBDebugLine Line;
			Line.Label = FString::Printf(TEXT("  [%d]"), i);
			FString TagStr = Behavior->GetBehaviorTag().ToString();
			Line.Value = FString::Printf(TEXT("%s (Priority: %d)"), *TagStr, Behavior->GetDefinition() ? Behavior->GetDefinition()->StackPriority : 0);
			OutDebugLines.Add(Line);
		}
	}

	// Deferred Entries/Exits queues
	if (DeferredEntries.Num() > 0)
	{
		FSBDebugLine DeferredHeader;
		DeferredHeader.Label = FString::Printf(TEXT("Deferred Entries (%d)"), DeferredEntries.Num());
		DeferredHeader.bIsHeader = true;
		OutDebugLines.Add(DeferredHeader);

		for (USBGameplayBehavior* Behavior : DeferredEntries)
		{
			if (Behavior)
			{
				FSBDebugLine Line;
				Line.Label = TEXT("  Pending Entry");
				Line.Value = Behavior->GetBehaviorTag().ToString();
				OutDebugLines.Add(Line);
			}
		}
	}

	if (DeferredExits.Num() > 0)
	{
		FSBDebugLine DeferredHeader;
		DeferredHeader.Label = FString::Printf(TEXT("Deferred Exits (%d)"), DeferredExits.Num());
		DeferredHeader.bIsHeader = true;
		OutDebugLines.Add(DeferredHeader);

		for (USBGameplayBehavior* Behavior : DeferredExits)
		{
			if (Behavior)
			{
				FSBDebugLine Line;
				Line.Label = TEXT("  Pending Exit");
				Line.Value = Behavior->GetBehaviorTag().ToString();
				OutDebugLines.Add(Line);
			}
		}
	}
}
