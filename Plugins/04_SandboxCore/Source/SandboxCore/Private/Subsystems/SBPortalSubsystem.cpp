// Copyright 2026 João Santos. All Rights Reserved.
#include "Subsystems/SBPortalSubsystem.h"
#include "Actors/SBPortalActor.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/SBCharacterInterface.h"
#include "Interfaces/SBStateComponentInterface.h"

USBPortalSubsystem::USBPortalSubsystem()
{
}

void USBPortalSubsystem::RegisterPortal(ASBPortalActor* Portal)
{
	if (Portal && !RegisteredPortals.Contains(Portal))
	{
		RegisteredPortals.Add(Portal);
	}
}

void USBPortalSubsystem::UnregisterPortal(ASBPortalActor* Portal)
{
	if (Portal)
	{
		RegisteredPortals.Remove(Portal);
	}
}

ASBPortalActor* USBPortalSubsystem::FindPortalByTag(FGameplayTag PortalTag) const
{
	if (!PortalTag.IsValid()) return nullptr;

	for (const auto& WeakPortal : RegisteredPortals)
	{
		if (ASBPortalActor* Portal = WeakPortal.Get())
		{
			if (Portal->PortalInfo.PortalTag == PortalTag)
			{
				return Portal;
			}
		}
	}
	return nullptr;
}

bool USBPortalSubsystem::RequestTeleport(AActor* Actor, const FSBPortalDestination& Destination, FGameplayTag SourcePortalTag)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	// 1. Verificação de chave de acesso se requerida
	if (Destination.RequiredKeyItemTag.IsValid())
	{
		bool bHasKey = false;
		UActorComponent* StateComp = nullptr;
		if (Actor->Implements<USBCharacterInterface>())
		{
			StateComp = ISBCharacterInterface::Execute_GetStateComponent(Actor);
		}

		// Fallback por contrato: ISBCharacterInterface::GetStateComponent retorna nulo em
		// ASBCharacter (override nao registrado na reflexao). O contrato do proprio
		// componente resolve corretamente e e mais desacoplado (Principio 4).
		if (!StateComp)
		{
			StateComp = Actor->FindComponentByInterface(USBStateComponentInterface::StaticClass());
		}

		if (StateComp && StateComp->Implements<USBStateComponentInterface>())
		{
			bHasKey = ISBStateComponentInterface::Execute_HasTag(StateComp, Destination.RequiredKeyItemTag);
		}

		if (!bHasKey)
		{
			UE_LOG(LogTemp, Warning, TEXT("USBPortalSubsystem::RequestTeleport: Actor %s lacks required key tag %s!"), *Actor->GetName(), *Destination.RequiredKeyItemTag.ToString());
			return false;
		}
	}

	// 2. Destino em outro nível
	if (Destination.TargetLevelName != NAME_None)
	{
		OnActorTeleported.Broadcast(Actor, SourcePortalTag, Destination.TargetPortalTag);
		UGameplayStatics::OpenLevel(GetWorld(), Destination.TargetLevelName);
		return true;
	}

	// 3. Destino por PortalTag no mesmo mapa
	FVector DestLocation = Destination.TargetLocation;
	FRotator DestRotation = Destination.TargetRotation;

	if (Destination.TargetPortalTag.IsValid())
	{
		ASBPortalActor* DestPortal = FindPortalByTag(Destination.TargetPortalTag);
		if (!DestPortal)
		{
			UE_LOG(LogTemp, Warning, TEXT("USBPortalSubsystem::RequestTeleport: Target portal %s not found in world!"), *Destination.TargetPortalTag.ToString());
			return false;
		}

		if (DestPortal->PortalInfo.bIsLocked || !DestPortal->PortalInfo.bIsOpen)
		{
			UE_LOG(LogTemp, Warning, TEXT("USBPortalSubsystem::RequestTeleport: Target portal %s is locked or closed!"), *Destination.TargetPortalTag.ToString());
			return false;
		}

		DestLocation = DestPortal->GetTeleportSpawnLocation();
		DestRotation = DestPortal->GetActorRotation();
	}

	// 4. Executa o teleporte local
	bool bSuccess = Actor->TeleportTo(DestLocation, DestRotation, false, true);
	if (bSuccess)
	{
		OnActorTeleported.Broadcast(Actor, SourcePortalTag, Destination.TargetPortalTag);
	}

	return bSuccess;
}

void USBPortalSubsystem::SetPortalLocked(FGameplayTag PortalTag, bool bLocked)
{
	if (ASBPortalActor* Portal = FindPortalByTag(PortalTag))
	{
		Portal->PortalInfo.bIsLocked = bLocked;
		OnPortalStateChanged.Broadcast(PortalTag, Portal->PortalInfo.bIsOpen, Portal->PortalInfo.bIsLocked);
	}
}

void USBPortalSubsystem::SetPortalOpen(FGameplayTag PortalTag, bool bOpen)
{
	if (ASBPortalActor* Portal = FindPortalByTag(PortalTag))
	{
		Portal->PortalInfo.bIsOpen = bOpen;
		OnPortalStateChanged.Broadcast(PortalTag, Portal->PortalInfo.bIsOpen, Portal->PortalInfo.bIsLocked);
	}
}

bool USBPortalSubsystem::IsPortalAvailable(FGameplayTag PortalTag) const
{
	if (ASBPortalActor* Portal = FindPortalByTag(PortalTag))
	{
		return Portal->PortalInfo.bIsOpen && !Portal->PortalInfo.bIsLocked;
	}
	return false;
}
