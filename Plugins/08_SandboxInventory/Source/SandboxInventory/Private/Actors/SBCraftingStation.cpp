// Copyright 2026 João Santos. All Rights Reserved.
// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/SBCraftingStation.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBInventoryComponent.h"
#include "Net/UnrealNetwork.h"

ASBCraftingStation::ASBCraftingStation()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetReplicates(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(100.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MaxInteractionDistance = 300.0f;
}

void ASBCraftingStation::BeginPlay()
{
	Super::BeginPlay();
}

void ASBCraftingStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		return;
	}

	for (int32 i = ActiveInteractors.Num() - 1; i >= 0; --i)
	{
		AActor* Interactor = ActiveInteractors[i].Get();
		if (!Interactor)
		{
			RemoveInteractorAt(i);
			continue;
		}

		float Distance = FVector::Dist(GetActorLocation(), Interactor->GetActorLocation());
		if (Distance > MaxInteractionDistance)
		{
			StopInteracting(Interactor);
		}
	}
}

bool ASBCraftingStation::CanInteract_Implementation(AActor* Interactor) const
{
	if (!Interactor)
	{
		return false;
	}

	// Permite interagir se possuir componente de inventário
	USBInventoryComponent* InvComp = Interactor->FindComponentByClass<USBInventoryComponent>();
	return InvComp != nullptr;
}

FText ASBCraftingStation::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	FString StationName = StationTag.IsValid() ? StationTag.GetTagName().ToString() : TEXT("Bancada");
	// Remove prefixos comuns de tags para exibição limpa (ex: State.Cooldown. -> "")
	StationName.ReplaceInline(TEXT("State.Station."), TEXT(""));
	StationName.ReplaceInline(TEXT("Crafting.Station."), TEXT(""));
	return FText::FromString(FString::Printf(TEXT("Usar %s"), *StationName));
}

void ASBCraftingStation::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority() || !Interactor)
	{
		return;
	}

	USBStateComponent* StateComp = Interactor->FindComponentByClass<USBStateComponent>();
	if (StateComp && StationTag.IsValid())
	{
		StateComp->AddTag(StationTag);
	}

	if (!ActiveInteractors.Contains(Interactor))
	{
		ActiveInteractors.Add(Interactor);
	}
}

void ASBCraftingStation::StopInteracting(AActor* Interactor)
{
	if (!HasAuthority() || !Interactor)
	{
		return;
	}

	USBStateComponent* StateComp = Interactor->FindComponentByClass<USBStateComponent>();
	if (StateComp && StationTag.IsValid())
	{
		StateComp->RemoveTag(StationTag);
	}

	ActiveInteractors.Remove(Interactor);
}

void ASBCraftingStation::RemoveInteractorAt(int32 Index)
{
	ActiveInteractors.RemoveAtSwap(Index);
}
