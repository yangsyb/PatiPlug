#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPatitiTilingEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
