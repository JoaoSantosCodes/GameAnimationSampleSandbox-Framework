// Copyright 2026 João Santos. All Rights Reserved.
#include "Components/SBHitTraceComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SceneComponent.h"
#include "SBGameplayTags.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

USBHitTraceComponent::USBHitTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USBHitTraceComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
		if (!SourceComponent.IsValid())
		{
			SourceComponent = Owner->GetRootComponent();
		}
	}
}

void USBHitTraceComponent::OnShutdown_Implementation()
{
	StopHitTrace();
}

void USBHitTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsTracingActive)
	{
		PerformTraceStep();
	}
}

void USBHitTraceComponent::StartHitTrace(const FSBHitTraceSettings& Settings, USceneComponent* InSourceComponent)
{
	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (InSourceComponent)
	{
		SourceComponent = InSourceComponent;
	}
	else if (!SourceComponent.IsValid() && GetOwner())
	{
		SourceComponent = GetOwner()->GetRootComponent();
	}

	ActiveSettings = Settings;
	bIsTracingActive = true;
	HitActorsInCurrentSwing.Empty();
	PreviousSocketLocations.Empty();

	// Inicializa posições anteriores dos sockets
	if (SourceComponent.IsValid())
	{
		for (const FSBHitTraceSocketConfig& SocketConfig : ActiveSettings.Sockets)
		{
			FVector InitialLoc = SourceComponent->GetSocketLocation(SocketConfig.SocketName);
			PreviousSocketLocations.Add(SocketConfig.SocketName, InitialLoc);
		}
	}

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->AddTag(Tags.State_Combat_Attacking);
		CachedStateComp->AddTag(Tags.State_Combat_HitTraceActive);
	}

	OnHitTraceStarted.Broadcast(ActiveSettings.AttackTag);
}

void USBHitTraceComponent::StopHitTrace()
{
	if (!bIsTracingActive) return;

	bIsTracingActive = false;
	int32 TotalHits = HitActorsInCurrentSwing.Num();
	HitActorsInCurrentSwing.Empty();
	PreviousSocketLocations.Empty();

	const FSBGameplayTags& Tags = FSBGameplayTags::Get();
	if (CachedStateComp.IsValid())
	{
		CachedStateComp->RemoveTag(Tags.State_Combat_Attacking);
		CachedStateComp->RemoveTag(Tags.State_Combat_HitTraceActive);
	}

	OnHitTraceEnded.Broadcast(TotalHits);
}

void USBHitTraceComponent::SetSourceComponent(USceneComponent* InSourceComponent)
{
	SourceComponent = InSourceComponent;
}

void USBHitTraceComponent::PerformTraceStep()
{
	if (!bIsTracingActive || !GetWorld()) return;

	if (!SourceComponent.IsValid() && GetOwner())
	{
		SourceComponent = GetOwner()->GetRootComponent();
	}

	if (!SourceComponent.IsValid()) return;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	for (const FSBHitTraceSocketConfig& SocketConfig : ActiveSettings.Sockets)
	{
		FVector CurrentLoc = SourceComponent->GetSocketLocation(SocketConfig.SocketName);
		// A origem é uma posição válida do mundo, então FVector::ZeroVector não serve como
		// sentinela de "sem posição anterior": um ator em (0,0,0) teria a varredura colapsada
		// para uma amostra pontual no destino, ignorando todo o caminho percorrido.
		// StartHitTrace já semeia o mapa, então a ausência da chave é o único caso de "sem
		// histórico" — distinguido aqui por Find em vez de FindRef.
		const FVector* PrevLocPtr = PreviousSocketLocations.Find(SocketConfig.SocketName);
		FVector PrevLoc = PrevLocPtr ? *PrevLocPtr : CurrentLoc;

		TArray<FHitResult> OutHits;

		if (ActiveSettings.bUseSphereSweep)
		{
			FCollisionShape Shape = FCollisionShape::MakeSphere(SocketConfig.TraceRadius);
			GetWorld()->SweepMultiByChannel(
				OutHits,
				PrevLoc,
				CurrentLoc,
				FQuat::Identity,
				ActiveSettings.TraceChannel,
				Shape,
				Params
			);
		}
		else
		{
			GetWorld()->LineTraceMultiByChannel(
				OutHits,
				PrevLoc,
				CurrentLoc,
				ActiveSettings.TraceChannel,
				Params
			);
		}

		for (const FHitResult& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (IsValid(HitActor) && HitActor != GetOwner() && !HitActorsInCurrentSwing.Contains(HitActor))
			{
				HitActorsInCurrentSwing.Add(HitActor);
				OnMeleeHit.Broadcast(HitActor, Hit, ActiveSettings.BaseDamage, ActiveSettings.AttackTag);
			}
		}

		PreviousSocketLocations.Add(SocketConfig.SocketName, CurrentLoc);
	}
}
