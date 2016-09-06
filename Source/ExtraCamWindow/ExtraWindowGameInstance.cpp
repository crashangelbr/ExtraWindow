#include "IExtraWindowPlugin.h"
#include "EngineModule.h"
#include "ExtraWindowGameInstance.h"
#include "ExtraWindowUI.h"

void UExtraWindowGameInstance::InitializeExtraWindowContext(APlayerController* controller)
{
	// Creates the world context. This should be the only WorldContext that ever gets created for this GameInstance.
	WorldContext = &GetEngine()->CreateNewWorldContext(EWorldType::None);
	WorldContext->OwningGameInstance = this;
	//WorldContext->RunAsDedicated = true;

	// create a ExtraWindow world
	UWorld* ExtraWindowWorld = UWorld::CreateWorld(EWorldType::None, false, "ExtraWindow");

	ExtraWindowWorld->SetGameInstance(this);
	GEngine->DestroyWorldContext(ExtraWindowWorld);
	//WorldContext->SetCurrentWorld( ExtraWindowWorld );

	ExtraWindowWorld->AddController(controller);

	Init();
}