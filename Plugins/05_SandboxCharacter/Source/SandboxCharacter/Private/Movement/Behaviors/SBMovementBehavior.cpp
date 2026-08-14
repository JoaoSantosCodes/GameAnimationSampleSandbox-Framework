#include "Movement/Behaviors/SBMovementBehavior.h"
#include "Components/SBMovementComponent.h"
#include "Movement/DataAssets/SBMovementBehaviorDefinition.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "GameFramework/Actor.h"

USBMovementBehavior::USBMovementBehavior()
{
}

void USBMovementBehavior::Initialize(USBBehaviorStackComponent* InStackComp, USBGameplayBehaviorDefinition* InDefinition)
{
	Super::Initialize(InStackComp, InDefinition);
	MovementComponent = Cast<USBMovementComponent>(InStackComp);
	MovementDefinition = Cast<USBMovementBehaviorDefinition>(InDefinition);

	MovementAttributeComponent = Cast<USBAttributeComponent>(USBGameplayBehavior::CachedAttributeComponent);
	MovementStateComponent = Cast<USBStateComponent>(USBGameplayBehavior::CachedStateComponent);
}

bool USBMovementBehavior::CanEnter_Implementation(const FSBBehaviorContext& Context) const
{
	if (!MovementDefinition)
	{
		return false;
	}

	if (MovementStateComponent)
	{
		// Verifica se temos as tags necessárias e se não estamos bloqueados
		if (!MovementStateComponent->HasAll(MovementDefinition->RequiredTags))
		{
			return false;
		}

		if (MovementStateComponent->HasAny(MovementDefinition->BlockedTags))
		{
			return false;
		}
	}

	return true;
}

bool USBMovementBehavior::CanExit_Implementation(const FSBBehaviorContext& Context) const
{
	return true;
}

void USBMovementBehavior::Enter_Implementation(const FSBBehaviorContext& Context)
{
}

void USBMovementBehavior::Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context)
{
}

void USBMovementBehavior::Exit_Implementation(const FSBBehaviorContext& Context)
{
}

int32 USBMovementBehavior::GetStackPriority() const
{
	return MovementDefinition ? MovementDefinition->StackPriority : 0;
}

void USBMockReentrantBehavior::Exit_Implementation(const FSBBehaviorContext& Context)
{
	Super::Exit_Implementation(Context);
	if (OwnerMC && TagToRequestOnExit.IsValid())
	{
		OwnerMC->RequestBehavior(TagToRequestOnExit);
	}
}
