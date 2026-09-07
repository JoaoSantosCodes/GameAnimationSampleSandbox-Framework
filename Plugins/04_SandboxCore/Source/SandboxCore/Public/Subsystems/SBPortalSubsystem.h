// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SBPortalTypes.h"
#include "SBPortalSubsystem.generated.h"

class ASBPortalActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBOnActorTeleported, AActor*, Actor, FGameplayTag, FromPortalTag, FGameplayTag, ToPortalTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSBOnPortalStateChanged, FGameplayTag, PortalTag, bool, bIsOpen, bool, bIsLocked);

/**
 * Subsistema de gerenciamento, registro e teleporte de Portais e Waystones
 */
UCLASS(BlueprintType)
class SANDBOXCORE_API USBPortalSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	USBPortalSubsystem();

	// Registro de Portais
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Portal")
	void RegisterPortal(ASBPortalActor* Portal);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Portal")
	void UnregisterPortal(ASBPortalActor* Portal);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Portal")
	ASBPortalActor* FindPortalByTag(FGameplayTag PortalTag) const;

	// Solicitação e execução de teleporte
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Portal")
	bool RequestTeleport(AActor* Actor, const FSBPortalDestination& Destination, FGameplayTag SourcePortalTag = FGameplayTag());

	// Controle de estado
	UFUNCTION(BlueprintCallable, Category = "Sandbox|Portal")
	void SetPortalLocked(FGameplayTag PortalTag, bool bLocked);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Portal")
	void SetPortalOpen(FGameplayTag PortalTag, bool bOpen);

	UFUNCTION(BlueprintPure, Category = "Sandbox|Portal")
	bool IsPortalAvailable(FGameplayTag PortalTag) const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Portal")
	FSBOnActorTeleported OnActorTeleported;

	UPROPERTY(BlueprintAssignable, Category = "Sandbox|Portal")
	FSBOnPortalStateChanged OnPortalStateChanged;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<ASBPortalActor>> RegisteredPortals;
};
