#pragma once

#include "Modules/ModuleManager.h"

class FSandboxCombatModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
