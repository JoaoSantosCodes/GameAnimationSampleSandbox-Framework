#include "Components/SBInteractionComponent.h"
#include "Components/SBStateComponent.h"
#include "Interfaces/SBInteractableInterface.h"
#include "Subsystems/SBEventSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

USBInteractionComponent::USBInteractionComponent()
	: Super(FObjectInitializer::Get())
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBInteractionComponent::OnInitialize_Implementation()
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		CachedStateComponent = Owner->FindComponentByClass<USBStateComponent>();
	}
}

void USBInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	APawn* PawnOwner = Cast<APawn>(Owner);
	const bool bIsLocallyControlled = GIsAutomationTesting || (PawnOwner && PawnOwner->IsLocallyControlled());

	UE_LOG(LogTemp, Warning, TEXT("TickComponent: Owner=%s, PawnOwner=%s, bIsLocallyControlled=%d, GIsAutomationTesting=%d"),
		Owner ? *Owner->GetName() : TEXT("nullptr"),
		PawnOwner ? *PawnOwner->GetName() : TEXT("nullptr"),
		bIsLocallyControlled,
		GIsAutomationTesting);

	if (bIsLocallyControlled)
	{
		ScanForInteractables();
	}

	if (bIsHoldingInteraction && CurrentInteractableActor)
	{
		// Validação física contínua de distância
		float DistSq = FVector::DistSquared(Owner->GetActorLocation(), CurrentInteractableActor->GetActorLocation());
		float MaxRangeWithTolerance = InteractionRange + 50.0f;
		if (DistSq > FMath::Square(MaxRangeWithTolerance))
		{
			StopInteract();
		}
		else
		{
			HoldProgressTime += DeltaTime;
			ProgressBroadcastAccumulator += DeltaTime;

			// Broadcast do evento de progresso com throttle de 60 Hz
			if (ProgressBroadcastAccumulator >= 0.01667f)
			{
				ProgressBroadcastAccumulator = 0.0f;
				if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
				{
					USBInteractionProgressEventPayload* Payload = NewObject<USBInteractionProgressEventPayload>(this);
					Payload->TargetPawn = Cast<APawn>(GetOwner());
					Payload->InteractableActor = CurrentInteractableActor;
					Payload->ProgressPercent = GetHoldProgressPercent();

					FGameplayTag ProgressTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Progress"));
					EventSubsystem->PublishEvent(ProgressTag, Payload);
				}
			}

			if (HoldProgressTime >= CurrentHoldDuration)
			{
				// Conclui hold no cliente local
				bIsHoldingInteraction = false;
				HoldProgressTime = 0.0f;

				if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
				{
					FGameplayTag CompletedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Completed"));
					EventSubsystem->PublishEvent(CompletedTag, CurrentInteractableActor);
				}

				if (!Owner->HasAuthority())
				{
					ServerCompleteInteract(CurrentInteractableActor);
				}
				else
				{
					ServerCompleteInteract_Implementation(CurrentInteractableActor);
				}

				if (CachedStateComponent)
				{
					FGameplayTag InteractingTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Interacting"));
					CachedStateComponent->RemoveTag(InteractingTag);
				}
			}
		}
	}
}

void USBInteractionComponent::ScanForInteractables()
{
	if (bIsHoldingInteraction)
	{
		return;
	}

	AActor* Owner = GetOwner();
	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	FVector Start, DirectionVector;
	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (PC)
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
		Start = ViewLocation;
		DirectionVector = ViewRotation.Vector();
	}
	else
	{
		FRotator EyeRotation;
		PawnOwner->GetActorEyesViewPoint(Start, EyeRotation);
		DirectionVector = EyeRotation.Vector();
	}

	FVector End = Start + DirectionVector * InteractionRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	FHitResult HitResult;
	bool bHit = false;

	if (bUseLineTrace)
	{
		bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, TraceChannel, QueryParams);
	}
	else
	{
		bHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(TraceRadius), QueryParams);
	}

	AActor* HitActor = bHit ? HitResult.GetActor() : nullptr;
	AActor* NewInteractable = nullptr;

	if (HitActor && HitActor->Implements<USBInteractableInterface>())
	{
		if (Target_CanInteract(HitActor))
		{
			NewInteractable = HitActor;
		}
	}

	if (NewInteractable != CurrentInteractableActor)
	{
		if (CurrentInteractableActor)
		{
			if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
			{
				FGameplayTag ClearedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Cleared"));
				EventSubsystem->PublishEvent(ClearedTag, CurrentInteractableActor);
			}
		}

		CurrentInteractableActor = NewInteractable;

		if (CurrentInteractableActor)
		{
			if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
			{
				USBInteractionAvailableEventPayload* Payload = NewObject<USBInteractionAvailableEventPayload>(this);
				Payload->TargetPawn = Cast<APawn>(GetOwner());
				Payload->InteractableActor = CurrentInteractableActor;
				Payload->PromptText = Target_GetInteractionPrompt(CurrentInteractableActor);
				Payload->Duration = Target_GetInteractionDuration(CurrentInteractableActor);

				FGameplayTag AvailableTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Available"));
				EventSubsystem->PublishEvent(AvailableTag, Payload);
			}
		}
	}
}

void USBInteractionComponent::StartInteract()
{
	if (!CurrentInteractableActor) return;

	AActor* Owner = GetOwner();
	float Duration = Target_GetInteractionDuration(CurrentInteractableActor);

	if (Duration > 0.0f)
	{
		bIsHoldingInteraction = true;
		HoldProgressTime = 0.0f;
		CurrentHoldDuration = Duration;

		if (CachedStateComponent)
		{
			FGameplayTag InteractingTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Interacting"));
			CachedStateComponent->AddTag(InteractingTag);
		}

		if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
		{
			FGameplayTag StartedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Started"));
			EventSubsystem->PublishEvent(StartedTag, CurrentInteractableActor);
		}

		if (!Owner->HasAuthority())
		{
			ServerStartInteract(CurrentInteractableActor);
		}
		else
		{
			ServerStartInteract_Implementation(CurrentInteractableActor);
		}
	}
	else
	{
		if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
		{
			FGameplayTag CompletedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Completed"));
			EventSubsystem->PublishEvent(CompletedTag, CurrentInteractableActor);
		}

		if (!Owner->HasAuthority())
		{
			ServerStartInteract(CurrentInteractableActor);
		}
		else
		{
			ServerStartInteract_Implementation(CurrentInteractableActor);
		}
	}
}

void USBInteractionComponent::StopInteract()
{
	if (bIsHoldingInteraction)
	{
		bIsHoldingInteraction = false;
		HoldProgressTime = 0.0f;

		if (CachedStateComponent)
		{
			FGameplayTag InteractingTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Interacting"));
			CachedStateComponent->RemoveTag(InteractingTag);
		}

		if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
		{
			FGameplayTag ClearedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Cleared"));
			EventSubsystem->PublishEvent(ClearedTag, CurrentInteractableActor);
		}

		AActor* Owner = GetOwner();
		if (!Owner->HasAuthority())
		{
			ServerStopInteract();
		}
		else
		{
			ServerStopInteract_Implementation();
		}
	}
}

float USBInteractionComponent::GetHoldProgressPercent() const
{
	if (CurrentHoldDuration > 0.0f)
	{
		return FMath::Clamp(HoldProgressTime / CurrentHoldDuration, 0.0f, 1.0f);
	}
	return 0.0f;
}

bool USBInteractionComponent::ServerStartInteract_Validate(AActor* Target)
{
	if (!InteractionRPCLimiter.AllowRPC(GetWorld(), 10.0f))
	{
		return false;
	}

	if (!Target)
	{
		return false;
	}

	if (!Target->GetClass()->ImplementsInterface(USBInteractableInterface::StaticClass()))
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Target->GetActorLocation());
	float LimitSq = FMath::Square(InteractionRange + 150.0f);
	if (DistSq > LimitSq)
	{
		return false;
	}

	return true;
}

void USBInteractionComponent::ServerStartInteract_Implementation(AActor* Target)
{
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerStartInteract: Target is nullptr"));
		ClientCancelInteraction();
		return;
	}
	if (!Target->Implements<USBInteractableInterface>())
	{
		ClientCancelInteraction();
		return;
	}

	AActor* Owner = GetOwner();
	float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Target->GetActorLocation());
	float LimitSq = FMath::Square(InteractionRange + 100.0f);
	if (DistSq > LimitSq)
	{
		ClientCancelInteraction();
		return;
	}

	if (Target_IsInteractionLocked(Target))
	{
		ClientCancelInteraction();
		return;
	}

	Target_LockInteraction(Target);

	float Duration = Target_GetInteractionDuration(Target);

	ActiveServerInteraction.TargetActor = Target;
	ActiveServerInteraction.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	ActiveServerInteraction.bIsHold = (Duration > 0.0f);

	if (!ActiveServerInteraction.bIsHold)
	{
		if (Owner->HasAuthority())
		{
			Target_Interact(Target);
		}
		Target_UnlockInteraction(Target);
		ActiveServerInteraction.TargetActor = nullptr;
	}
	else
	{
		if (CachedStateComponent)
		{
			FGameplayTag InteractingTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Interacting"));
			CachedStateComponent->AddTag(InteractingTag);
		}
	}
}

bool USBInteractionComponent::ServerStopInteract_Validate()
{
	if (!InteractionRPCLimiter.AllowRPC(GetWorld(), 10.0f))
	{
		return false;
	}
	return true;
}

void USBInteractionComponent::ServerStopInteract_Implementation()
{
	if (ActiveServerInteraction.TargetActor.IsValid())
	{
		AActor* Target = ActiveServerInteraction.TargetActor.Get();
		AActor* Owner = GetOwner();

		Target_UnlockInteraction(Target);
		ActiveServerInteraction.TargetActor = nullptr;

		if (CachedStateComponent)
		{
			FGameplayTag InteractingTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Interacting"));
			CachedStateComponent->RemoveTag(InteractingTag);
		}
	}
}

bool USBInteractionComponent::ServerCompleteInteract_Validate(AActor* Target)
{
	if (!InteractionRPCLimiter.AllowRPC(GetWorld(), 10.0f))
	{
		return false;
	}

	if (!Target)
	{
		return false;
	}

	if (Target != ActiveServerInteraction.TargetActor.Get())
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Target->GetActorLocation());
	float LimitSq = FMath::Square(InteractionRange + 150.0f);
	if (DistSq > LimitSq)
	{
		return false;
	}

	return true;
}

void USBInteractionComponent::ServerCompleteInteract_Implementation(AActor* Target)
{
	if (!Target || Target != ActiveServerInteraction.TargetActor.Get())
	{
		ClientCancelInteraction();
		return;
	}

	AActor* Owner = GetOwner();
	float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Target->GetActorLocation());
	if (DistSq > FMath::Square(InteractionRange + 100.0f))
	{
		ClientCancelInteraction();
		return;
	}

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	float Duration = Target_GetInteractionDuration(Target);
	float Elapsed = CurrentTime - ActiveServerInteraction.StartTime;

	if (Elapsed < (Duration - 0.1f))
	{
		ClientCancelInteraction();
		return;
	}

	if (Owner->HasAuthority())
	{
		Target_Interact(Target);
	}

	Target_UnlockInteraction(Target);
	ActiveServerInteraction.TargetActor = nullptr;

	if (CachedStateComponent)
	{
		FGameplayTag InteractingTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Interacting"));
		CachedStateComponent->RemoveTag(InteractingTag);
	}
}

void USBInteractionComponent::ClientCancelInteraction_Implementation()
{
	bIsHoldingInteraction = false;
	HoldProgressTime = 0.0f;

	if (CachedStateComponent)
	{
		FGameplayTag InteractingTag = FGameplayTag::RequestGameplayTag(TEXT("State.Character.Interacting"));
		CachedStateComponent->RemoveTag(InteractingTag);
	}

	if (USBEventSubsystem* EventSubsystem = GetEventSubsystem())
	{
		FGameplayTag ClearedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Cleared"));
		EventSubsystem->PublishEvent(ClearedTag, CurrentInteractableActor);
	}
}

USBEventSubsystem* USBInteractionComponent::GetEventSubsystem() const
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<USBEventSubsystem>() : nullptr;
}

bool USBInteractionComponent::Target_CanInteract(AActor* Target) const
{
	return RouteInterfaceCall<bool>(
		Target,
		[Target, this]() { return ISBInteractableInterface::Execute_CanInteract(Target, GetOwner()); },
		[this](ISBInteractableInterface* I) { return I->CanInteract_Implementation(GetOwner()); },
		false
	);
}

void USBInteractionComponent::Target_Interact(AActor* Target)
{
	RouteInterfaceCallVoid(
		Target,
		[Target, this]() { ISBInteractableInterface::Execute_Interact(Target, GetOwner()); },
		[this](ISBInteractableInterface* I) { I->Interact_Implementation(GetOwner()); }
	);
}

FText USBInteractionComponent::Target_GetInteractionPrompt(AActor* Target) const
{
	return RouteInterfaceCall<FText>(
		Target,
		[Target, this]() { return ISBInteractableInterface::Execute_GetInteractionPrompt(Target, GetOwner()); },
		[this](ISBInteractableInterface* I) { return I->GetInteractionPrompt_Implementation(GetOwner()); },
		FText::GetEmpty()
	);
}

float USBInteractionComponent::Target_GetInteractionDuration(AActor* Target) const
{
	return RouteInterfaceCall<float>(
		Target,
		[Target, this]() { return ISBInteractableInterface::Execute_GetInteractionDuration(Target, GetOwner()); },
		[this](ISBInteractableInterface* I) { return I->GetInteractionDuration_Implementation(GetOwner()); },
		0.0f
	);
}

bool USBInteractionComponent::Target_IsInteractionLocked(AActor* Target) const
{
	return RouteInterfaceCall<bool>(
		Target,
		[Target, this]() { return ISBInteractableInterface::Execute_IsInteractionLocked(Target, GetOwner()); },
		[this](ISBInteractableInterface* I) { return I->IsInteractionLocked_Implementation(GetOwner()); },
		false
	);
}

void USBInteractionComponent::Target_LockInteraction(AActor* Target)
{
	RouteInterfaceCallVoid(
		Target,
		[Target, this]() { ISBInteractableInterface::Execute_LockInteraction(Target, GetOwner()); },
		[this](ISBInteractableInterface* I) { I->LockInteraction_Implementation(GetOwner()); }
	);
}

void USBInteractionComponent::Target_UnlockInteraction(AActor* Target)
{
	RouteInterfaceCallVoid(
		Target,
		[Target, this]() { ISBInteractableInterface::Execute_UnlockInteraction(Target, GetOwner()); },
		[this](ISBInteractableInterface* I) { I->UnlockInteraction_Implementation(GetOwner()); }
	);
}
