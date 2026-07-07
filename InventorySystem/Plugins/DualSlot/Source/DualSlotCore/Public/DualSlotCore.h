// Copyright Under the Bay Studios. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

DUALSLOTCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogDualSlot, Log, All);

class FDualSlotCoreModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
