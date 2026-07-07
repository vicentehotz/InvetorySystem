// Copyright Under the Bay Studios. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FDualSlotUIModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
