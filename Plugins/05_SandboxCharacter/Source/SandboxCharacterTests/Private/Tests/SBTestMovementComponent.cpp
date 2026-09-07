// Copyright 2026 João Santos. All Rights Reserved.
#include "Tests/SBTestMovementComponent.h"

void USBTestMovementComponent::ClientStopBehavior_Implementation(FGameplayTag BehaviorTag)
{
	LastClientStopBehaviorTag = BehaviorTag;
	Super::ClientStopBehavior_Implementation(BehaviorTag);
}
