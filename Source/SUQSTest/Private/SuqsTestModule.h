#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSuqsTestModule, All, All)

class FSuqsTestModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};