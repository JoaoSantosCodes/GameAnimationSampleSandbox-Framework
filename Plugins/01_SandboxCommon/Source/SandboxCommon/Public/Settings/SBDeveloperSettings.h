#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SBDeveloperSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Sandbox Framework"))
class SANDBOXCOMMON_API USBDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USBDeveloperSettings();

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay")
	bool bEnableDebugLogs = true;

	UPROPERTY(Config, EditAnywhere, Category = "Gameplay")
	bool bShowDebugDraws = false;
};
