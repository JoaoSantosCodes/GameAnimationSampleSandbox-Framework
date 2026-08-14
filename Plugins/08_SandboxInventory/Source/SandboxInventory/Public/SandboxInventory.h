#pragma once

#include "Modules/ModuleManager.h"

class FSandboxInventoryModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
