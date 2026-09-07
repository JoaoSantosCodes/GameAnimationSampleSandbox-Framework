#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "SBModularActors.generated.h"

/**
 * Classes-base que registram o ator no UGameFrameworkComponentManager, para que Game
 * Feature Plugins possam injetar componentes nele em tempo de execucao.
 *
 * Existem para substituir as classes AModular* do plugin ModularGameplayActors, que
 * acompanha o projeto Lyra e nao a engine — nenhum projeto que receba o framework teria
 * esse plugin. Estas dependem apenas de ModularGameplay, que e plugin de engine.
 */

UCLASS(Blueprintable)
class SANDBOXCORE_API ASBModularPawn : public APawn
{
	GENERATED_BODY()

public:
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

UCLASS(Blueprintable)
class SANDBOXCORE_API ASBModularCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

UCLASS(Blueprintable)
class SANDBOXCORE_API ASBModularPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void PreInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** O controller so fica util depois de receber o player; o evento de pronto sai daqui. */
	virtual void ReceivedPlayer() override;
	virtual void PlayerTick(float DeltaTime) override;
};

UCLASS(Blueprintable)
class SANDBOXCORE_API ASBModularPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Reset() override;

	/** Preserva o estado dos componentes na troca de PlayerState (seamless travel). */
	virtual void CopyProperties(APlayerState* NewPlayerState) override;
};

UCLASS(Blueprintable)
class SANDBOXCORE_API ASBModularGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

UCLASS(Blueprintable)
class SANDBOXCORE_API ASBModularGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASBModularGameModeBase();
};
