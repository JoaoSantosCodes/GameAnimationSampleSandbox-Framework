#include "Character/SBCharacter.h"
#include "DataAssets/SBPawnDataAsset.h"
#include "Components/SBComponentFactory.h"
#include "Interfaces/SBCharacterInterface.h"
#include "Interfaces/SBComponentInterface.h"
#include "Utilities/SBLogCategories.h"

// We forward-declare/include components to access their static classes
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "Components/SBAbilityComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

ASBCharacter::ASBCharacter()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	}
}

UObject* ASBCharacter::GetPawnData_Implementation() const
{
	return PawnData;
}

UActorComponent* ASBCharacter::GetAttributeComponent_Implementation() const
{
	return FindComponentByClass<USBAttributeComponent>();
}

UActorComponent* ASBCharacter::GetStateComponent_Implementation() const
{
	return FindComponentByClass<USBStateComponent>();
}

UActorComponent* ASBCharacter::GetAbilityComponent_Implementation() const
{
	return FindComponentByClass<USBAbilityComponent>();
}

void ASBCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeFromPawnData();
}

void ASBCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeFromPawnData();
}

void ASBCharacter::InitializeFromPawnData()
{
	if (!PawnData)
	{
		UE_LOG(LogSandboxCharacter, Warning, TEXT("No PawnData set on character %s!"), *GetName());
		return;
	}

	// 1. Spawning dynamic components from ComponentSet
	if (PawnData->ComponentSet)
	{
		USBComponentFactory::InitializeComponentsFromSet(this, PawnData->ComponentSet);
	}

	// 2. Setting up mesh and animations if provided
	if (PawnData->Mesh)
	{
		GetMesh()->SetSkeletalMesh(PawnData->Mesh);
	}
	if (PawnData->AnimClass)
	{
		GetMesh()->SetAnimInstanceClass(PawnData->AnimClass);
	}

	// 3. Adding default tags from PawnData to StateComponent
	if (USBStateComponent* StateComp = Cast<USBStateComponent>(GetStateComponent_Implementation()))
	{
		for (const FGameplayTag& Tag : PawnData->DefaultTags)
		{
			StateComp->AddTag(Tag);
		}
	}

	UE_LOG(LogSandboxCharacter, Log, TEXT("Successfully initialized character %s from PawnData %s"), *GetName(), *PawnData->GetName());
}

bool ASBCharacter::IsLocallyControlled() const
{
	return IsPlayerControlled() && Super::IsLocallyControlled();
}

void ASBCharacter::Test_Possess(AController* NewController)
{
	if (GIsAutomationTesting && NewController)
	{
		PossessedBy(NewController);
	}
}

void ASBCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (USBAbilityComponent* AbilityComp = Cast<USBAbilityComponent>(GetAbilityComponent_Implementation()))
	{
		AbilityComp->BindInputActions(PlayerInputComponent);
	}
}

void ASBCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TArray<UActorComponent*> Components;
	GetComponents(Components);
	for (UActorComponent* Comp : Components)
	{
		if (Comp && Comp->GetClass()->ImplementsInterface(USBComponentInterface::StaticClass()))
		{
			ISBComponentInterface::Execute_OnShutdown(Comp);
		}
	}

	Super::EndPlay(EndPlayReason);
}
