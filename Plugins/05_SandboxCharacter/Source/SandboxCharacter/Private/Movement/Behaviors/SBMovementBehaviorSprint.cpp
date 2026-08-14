#include "Movement/Behaviors/SBMovementBehaviorSprint.h"
#include "GameFramework/Character.h"
#include "Movement/DataAssets/SBMovementBehaviorDefinition.h"
#include "Components/SBAttributeComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"
// Forward inclusion do orquestrador
#include "Components/SBMovementComponent.h"

USBMovementBehaviorSprint::USBMovementBehaviorSprint()
{
}

bool USBMovementBehaviorSprint::CanEnter_Implementation(const FSBBehaviorContext& Context) const
{
	// Chama a validação base de tags necessárias/bloqueadas
	if (!Super::CanEnter_Implementation(Context))
	{
		return false;
	}

	// Verifica se temos stamina suficiente para iniciar a corrida
	if (MovementAttributeComponent)
	{
		float StaminaValue = MovementAttributeComponent->GetAttributeValue(FSBGameplayTags::Get().Attribute_Stamina);
		if (StaminaValue <= 0.0f)
		{
			return false;
		}
	}

	return true;
}

void USBMovementBehaviorSprint::Enter_Implementation(const FSBBehaviorContext& Context)
{
	Super::Enter_Implementation(Context);

	// Reinicializa os dados transientes de runtime
	SprintRuntimeData.SprintDuration = 0.0f;

	// Aplica a tag de estado ao componente de estado
	if (MovementStateComponent)
	{
		MovementStateComponent->AddTag(FSBGameplayTags::Get().State_Character_Sprinting);
	}

	// Registra os modificadores físicos no orquestrador
	if (MovementDefinition && MovementComponent)
	{
		MovementComponent->ApplyMovementModifiers(MovementDefinition->BehaviorTag, MovementDefinition->MovementModifiers);
	}
}

void USBMovementBehaviorSprint::Update_Implementation(float DeltaTime, const FSBBehaviorContext& Context)
{
	Super::Update_Implementation(DeltaTime, Context);

	SprintRuntimeData.SprintDuration += DeltaTime;

	if (MovementDefinition && MovementAttributeComponent)
	{
		float StaminaCost = MovementDefinition->StaminaCostPerSecond * DeltaTime;
		if (StaminaCost > 0.0f)
		{
			// Tenta consumir a stamina via componente de atributos
			bool bHasStamina = MovementAttributeComponent->TryConsumeAttribute(
				FSBGameplayTags::Get().Attribute_Stamina,
				StaminaCost,
				Context.GameplayContext ? Context.GameplayContext->Character.Get() : nullptr
			);

			// Se a stamina acabar, para o comportamento de corrida imediatamente
			if (!bHasStamina && MovementComponent)
			{
				MovementComponent->StopBehavior(MovementDefinition->BehaviorTag);
			}
		}
	}
}

void USBMovementBehaviorSprint::Exit_Implementation(const FSBBehaviorContext& Context)
{
	// Remove os modificadores físicos associados a este behavior no orquestrador
	if (MovementDefinition && MovementComponent)
	{
		MovementComponent->RemoveMovementModifiers(MovementDefinition->BehaviorTag);
	}

	// Remove a tag de estado correspondente
	if (MovementStateComponent)
	{
		MovementStateComponent->RemoveTag(FSBGameplayTags::Get().State_Character_Sprinting);
	}

	Super::Exit_Implementation(Context);
}
