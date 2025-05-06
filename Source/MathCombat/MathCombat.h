

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


/**
 * 
 */
class MATHCOMBAT_API MathCombat : public IModuleInterface
{
public:
	MathCombat();
	~MathCombat();
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
