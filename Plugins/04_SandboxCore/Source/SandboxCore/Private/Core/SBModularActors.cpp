#include "Core/SBModularActors.h"

#include "Components/ControllerComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/PlayerStateComponent.h"

namespace SBModularActors
{
	/** Registra o ator como receptor de componentes injetados por Game Features. */
	static void RegisterReceiver(AActor* Actor)
	{
		UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(Actor);
	}

	static void UnregisterReceiver(AActor* Actor)
	{
		UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(Actor);
	}

	/** Avisa que o ator esta pronto para receber a extensao. */
	static void NotifyActorReady(AActor* Actor)
	{
		UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
			Actor, UGameFrameworkComponentManager::NAME_GameActorReady);
	}
}

// ---------------------------------------------------------------------------- Pawn

void ASBModularPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	SBModularActors::RegisterReceiver(this);
}

void ASBModularPawn::BeginPlay()
{
	SBModularActors::NotifyActorReady(this);
	Super::BeginPlay();
}

void ASBModularPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SBModularActors::UnregisterReceiver(this);
	Super::EndPlay(EndPlayReason);
}

// ----------------------------------------------------------------------- Character

void ASBModularCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	SBModularActors::RegisterReceiver(this);
}

void ASBModularCharacter::BeginPlay()
{
	SBModularActors::NotifyActorReady(this);
	Super::BeginPlay();
}

void ASBModularCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SBModularActors::UnregisterReceiver(this);
	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------- PlayerController

void ASBModularPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	SBModularActors::RegisterReceiver(this);
}

void ASBModularPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SBModularActors::UnregisterReceiver(this);
	Super::EndPlay(EndPlayReason);
}

void ASBModularPlayerController::ReceivedPlayer()
{
	SBModularActors::NotifyActorReady(this);

	Super::ReceivedPlayer();

	TInlineComponentArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->ReceivedPlayer();
	}
}

void ASBModularPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	TInlineComponentArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->PlayerTick(DeltaTime);
	}
}

// ------------------------------------------------------------------- PlayerState

void ASBModularPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	SBModularActors::RegisterReceiver(this);
}

void ASBModularPlayerState::BeginPlay()
{
	SBModularActors::NotifyActorReady(this);
	Super::BeginPlay();
}

void ASBModularPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SBModularActors::UnregisterReceiver(this);
	Super::EndPlay(EndPlayReason);
}

void ASBModularPlayerState::Reset()
{
	Super::Reset();

	TInlineComponentArray<UPlayerStateComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UPlayerStateComponent* Component : ModularComponents)
	{
		Component->Reset();
	}
}

void ASBModularPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);

	if (!NewPlayerState)
	{
		return;
	}

	TInlineComponentArray<UPlayerStateComponent*> SourceComponents;
	GetComponents(SourceComponents);

	TInlineComponentArray<UPlayerStateComponent*> TargetComponents;
	NewPlayerState->GetComponents(TargetComponents);

	// O par correspondente e o componente de mesma classe e mesmo nome no destino.
	for (UPlayerStateComponent* Source : SourceComponents)
	{
		for (UPlayerStateComponent* Target : TargetComponents)
		{
			if (Target->GetClass() == Source->GetClass() && Target->GetFName() == Source->GetFName())
			{
				Source->CopyProperties(Target);
				break;
			}
		}
	}
}

// -------------------------------------------------------------------- GameState

void ASBModularGameStateBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	SBModularActors::RegisterReceiver(this);
}

void ASBModularGameStateBase::BeginPlay()
{
	SBModularActors::NotifyActorReady(this);
	Super::BeginPlay();
}

void ASBModularGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SBModularActors::UnregisterReceiver(this);
	Super::EndPlay(EndPlayReason);
}

// --------------------------------------------------------------------- GameMode

ASBModularGameModeBase::ASBModularGameModeBase()
{
	GameStateClass = ASBModularGameStateBase::StaticClass();
	PlayerControllerClass = ASBModularPlayerController::StaticClass();
	PlayerStateClass = ASBModularPlayerState::StaticClass();
	DefaultPawnClass = ASBModularPawn::StaticClass();
}
