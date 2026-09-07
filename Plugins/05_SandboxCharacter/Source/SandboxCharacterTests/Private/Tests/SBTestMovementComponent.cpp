#include "Tests/SBTestMovementComponent.h"

void USBTestMovementComponent::ClientStopBehavior_Implementation(FGameplayTag BehaviorTag)
{
	LastClientStopBehaviorTag = BehaviorTag;
	Super::ClientStopBehavior_Implementation(BehaviorTag);
}
