#include "Behaviors/SBGameplayBehavior.h"
#include "Behaviors/SBGameplayBehaviorDefinition.h"
#include "Interfaces/SBCharacterInterface.h"
#include "Interfaces/SBStateComponentInterface.h"
#include "GameFramework/Actor.h"
#include "Components/SBBehaviorStackComponent.h"

USBGameplayBehavior::USBGameplayBehavior()
{
}

void USBGameplayBehavior::Initialize(USBBehaviorStackComponent* InStackComp, USBGameplayBehaviorDefinition* InDefinition)
{
	StackComponent = InStackComp;
	Definition = InDefinition;

	if (InStackComp && InStackComp->GetOwner())
	{
		AActor* Owner = InStackComp->GetOwner();
		ISBCharacterInterface* CharInterface = Cast<ISBCharacterInterface>(Owner);
		if (CharInterface)
		{
			CachedAttributeComponent = CharInterface->GetAttributeComponent_Implementation();
			CachedStateComponent = CharInterface->GetStateComponent_Implementation();
		}
		else if (Owner->Implements<USBCharacterInterface>() || Owner->GetClass()->ImplementsInterface(USBCharacterInterface::StaticClass()))
		{
			CachedAttributeComponent = ISBCharacterInterface::Execute_GetAttributeComponent(Owner);
			CachedStateComponent = ISBCharacterInterface::Execute_GetStateComponent(Owner);
		}

	}
}

bool USBGameplayBehavior::CanEnter_Implementation(const FSBBehaviorContext& Context) const
{
	if (!Definition)
	{
		return false;
	}

	ISBStateComponentInterface* StateInterface = Cast<ISBStateComponentInterface>(CachedStateComponent);
	if (StateInterface)
	{
		if (!StateInterface->HasAll_Implementation(Definition->RequiredTags))
		{
			return false;
		}

		if (StateInterface->HasAny_Implementation(Definition->BlockedTags))
		{
			return false;
		}
	}
	else if (CachedStateComponent && (CachedStateComponent->Implements<USBStateComponentInterface>() || CachedStateComponent->GetClass()->ImplementsInterface(USBStateComponentInterface::StaticClass())))
	{
		// Verifica se temos as tags necessárias e se não estamos bloqueados
		if (!ISBStateComponentInterface::Execute_HasAll(CachedStateComponent, Definition->RequiredTags))
		{
			return false;
		}

		if (ISBStateComponentInterface::Execute_HasAny(CachedStateComponent, Definition->BlockedTags))
		{
			return false;
		}
	}

	return true;
}

bool USBGameplayBehavior::CanExit_Implementation(const FSBBehaviorContext& Context) const
{
	return true;
}

void USBGameplayBehavior::Enter_Implementation(const FSBBehaviorContext& Context)
{
}

void USBGameplayBehavior::Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context)
{
}

void USBGameplayBehavior::Exit_Implementation(const FSBBehaviorContext& Context)
{
}

int32 USBGameplayBehavior::GetStackPriority() const
{
	return Definition ? Definition->StackPriority : 0;
}

FGameplayTag USBGameplayBehavior::GetBehaviorTag() const
{
	return Definition ? Definition->BehaviorTag : FGameplayTag();
}
