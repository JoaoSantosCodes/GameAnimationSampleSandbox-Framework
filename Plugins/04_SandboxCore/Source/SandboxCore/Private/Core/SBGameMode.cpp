#include "Core/SBGameMode.h"
#include "Core/SBGameState.h"
#include "Core/SBPlayerController.h"
#include "Core/SBPlayerState.h"

ASBGameMode::ASBGameMode()
{
	GameStateClass = ASBGameState::StaticClass();
	PlayerControllerClass = ASBPlayerController::StaticClass();
	PlayerStateClass = ASBPlayerState::StaticClass();
}
