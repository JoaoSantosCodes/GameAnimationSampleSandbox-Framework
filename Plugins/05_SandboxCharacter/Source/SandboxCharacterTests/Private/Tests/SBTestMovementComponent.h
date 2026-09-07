// Copyright 2026 João Santos. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/SBMovementComponent.h"
#include "SBTestMovementComponent.generated.h"

/** Subclasse usada so pelos testes de rede, para observar o que chegou no cliente. */
UCLASS()
class USBTestMovementComponent : public USBMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayTag LastClientStopBehaviorTag;

	virtual void ClientStopBehavior_Implementation(FGameplayTag BehaviorTag) override;
};
