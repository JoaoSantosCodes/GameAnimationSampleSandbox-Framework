#include "Movement/Behaviors/SBMovementBehaviorCrouch.h"
#include "Components/SBStateComponent.h"
#include "Components/SBMovementComponent.h"
#include "SBGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USBMovementBehaviorCrouch::USBMovementBehaviorCrouch()
{
}

bool USBMovementBehaviorCrouch::CanEnter_Implementation(const FSBBehaviorContext& Context) const
{
	if (!Super::CanEnter_Implementation(Context))
	{
		return false;
	}

	if (!Context.GameplayContext || !Context.GameplayContext->Character)
	{
		return false;
	}

	ACharacter* Character = Context.GameplayContext->Character.Get();
	UCharacterMovementComponent* CharMove = Character->GetCharacterMovement();
	
	if (!CharMove || !CharMove->CanCrouchInCurrentState())
	{
		return false;
	}

	return true;
}

void USBMovementBehaviorCrouch::Enter_Implementation(const FSBBehaviorContext& Context)
{
	Super::Enter_Implementation(Context);

	if (Context.GameplayContext && Context.GameplayContext->Character)
	{
		ACharacter* Character = Context.GameplayContext->Character.Get();
		UCharacterMovementComponent* CharMove = Character->GetCharacterMovement();
		const USBMovementBehaviorCrouchDefinition* CrouchDef = Cast<USBMovementBehaviorCrouchDefinition>(MovementDefinition);

		if (!ensureMsgf(CrouchDef, TEXT("Crouch Behavior '%s' está configurado com um Definition incompatível! Esperava USBMovementBehaviorCrouchDefinition."), *GetName()))
		{
			return;
		}

		if (CharMove)
		{
			// Configura a altura do Crouch definida pelo designer no Data Asset
			CharMove->CrouchedHalfHeight = CrouchDef->CrouchedHalfHeight;
		}

		if (Character)
		{
			// Dispara o pipeline de Crouch nativo da engine (replicação, predição e ajuste suave de câmera)
			Character->Crouch();
		}
	}

	if (MovementStateComponent)
	{
		MovementStateComponent->AddTag(FSBGameplayTags::Get().State_Character_Crouching);
	}

	// Registra os modificadores físicos (ex: velocidade * 0.5) no orquestrador
	if (MovementDefinition && MovementComponent)
	{
		MovementComponent->ApplyMovementModifiers(MovementDefinition->BehaviorTag, MovementDefinition->MovementModifiers);
	}
}

void USBMovementBehaviorCrouch::Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context)
{
	Super::Update_Implementation(DeltaTime, Context);
}

void USBMovementBehaviorCrouch::Exit_Implementation(const FSBBehaviorContext& Context)
{
	// 1. Remove os modificadores físicos associados a este behavior no orquestrador
	if (MovementDefinition && MovementComponent)
	{
		MovementComponent->RemoveMovementModifiers(MovementDefinition->BehaviorTag);
	}

	if (Context.GameplayContext && Context.GameplayContext->Character)
	{
		ACharacter* Character = Context.GameplayContext->Character.Get();
		if (Character)
		{
			// Dispara o pipeline de UnCrouch nativo da engine (valida colisão no teto e restaura a altura suavemente)
			Character->UnCrouch();
		}
	}

	if (MovementStateComponent)
	{
		MovementStateComponent->RemoveTag(FSBGameplayTags::Get().State_Character_Crouching);
	}

	Super::Exit_Implementation(Context);
}
