#include "Components/SBAsyncParallelDataComponent.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "Subsystems/SBAsyncTaskManagerSubsystem.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

USBAsyncParallelDataComponent::USBAsyncParallelDataComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	FrontBufferValue = 0.0f;
	BackBufferValue = 0.0f;
	bIsAsyncWorkRunning = false;
}

void USBAsyncParallelDataComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	}

	SyncTags();
}

void USBAsyncParallelDataComponent::OnShutdown_Implementation()
{
	bIsAsyncWorkRunning = false;
}

void USBAsyncParallelDataComponent::SetFrontBufferValue(float Value)
{
	FrontBufferValue = Value;
	BackBufferValue = Value;
}

void USBAsyncParallelDataComponent::RequestAsyncCalculation(float Multiplier, float Additive)
{
	if (!GetWorld())
	{
		return;
	}

	USBAsyncTaskManagerSubsystem* AsyncSubsystem = GetWorld()->GetSubsystem<USBAsyncTaskManagerSubsystem>();
	if (!AsyncSubsystem)
	{
		return;
	}

	bIsAsyncWorkRunning = true;
	SyncTags();

	TArray<float> Inputs;
	Inputs.Add(FrontBufferValue);

	TWeakObjectPtr<USBAsyncParallelDataComponent> WeakThis(this);

	AsyncSubsystem->DispatchAsyncBatchCalculation(
		Inputs,
		[Multiplier, Additive](float Val) -> float
		{
			return (Val * Multiplier) + Additive;
		},
		[WeakThis](const TArray<float>& Results)
		{
			if (WeakThis.IsValid() && Results.Num() > 0)
			{
				WeakThis->BackBufferValue = Results[0];
				WeakThis->bIsAsyncWorkRunning = false;
				WeakThis->SyncTags();
				WeakThis->OnAsyncWorkFinished.Broadcast(WeakThis->BackBufferValue);
			}
		}
	);
}

void USBAsyncParallelDataComponent::CommitBackBuffer()
{
	FrontBufferValue = BackBufferValue;
}

void USBAsyncParallelDataComponent::SyncTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByInterface(USBStateComponentInterface::StaticClass());
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Async_DoubleBufferActive);

	if (bIsAsyncWorkRunning)
	{
		ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Async_TaskRunning);
		ISBStateComponentInterface::Execute_RemoveTag(CachedStateComp.Get(), Tags.State_Async_WorkCompleted);
	}
	else
	{
		ISBStateComponentInterface::Execute_RemoveTag(CachedStateComp.Get(), Tags.State_Async_TaskRunning);
		ISBStateComponentInterface::Execute_AddTag(CachedStateComp.Get(), Tags.State_Async_WorkCompleted);
	}
}
