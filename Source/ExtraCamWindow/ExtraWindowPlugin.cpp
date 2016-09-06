#include "IExtraWindowPlugin.h"

class FExtraWindowPlugin : public IExtraWindowPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FExtraWindowPlugin, ExtraWindow)

void FExtraWindowPlugin::StartupModule()
{

}

void FExtraWindowPlugin::ShutdownModule()
{

}