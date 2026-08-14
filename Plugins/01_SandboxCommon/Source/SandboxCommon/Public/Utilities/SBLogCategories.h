#pragma once

#include "CoreMinimal.h"

SANDBOXCOMMON_API DECLARE_LOG_CATEGORY_EXTERN(LogSandbox, Log, All);
SANDBOXCOMMON_API DECLARE_LOG_CATEGORY_EXTERN(LogSandboxCore, Log, All);
SANDBOXCOMMON_API DECLARE_LOG_CATEGORY_EXTERN(LogSandboxCharacter, Log, All);
SANDBOXCOMMON_API DECLARE_LOG_CATEGORY_EXTERN(LogSandboxUI, Log, All);
SANDBOXCOMMON_API DECLARE_LOG_CATEGORY_EXTERN(LogSandboxCombat, Log, All);
SANDBOXCOMMON_API DECLARE_LOG_CATEGORY_EXTERN(LogSandboxInventory, Log, All);

#define SB_LOG(Category, Verbosity, Format, ...) \
	UE_LOG(Category, Verbosity, Format, ##__VA_ARGS__)

#define SB_LOG_INFO(Format, ...) SB_LOG(LogSandbox, Log, Format, ##__VA_ARGS__)
#define SB_LOG_WARNING(Format, ...) SB_LOG(LogSandbox, Warning, Format, ##__VA_ARGS__)
#define SB_LOG_ERROR(Format, ...) SB_LOG(LogSandbox, Error, Format, ##__VA_ARGS__)
