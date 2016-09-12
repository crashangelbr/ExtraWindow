#include "IExtraWindowPlugin.h"
#include "EngineModule.h"
#include "ExtraWindowGameInstance.h"
#include "ExtraWindowUI.h"

void UExtraWindowUIGameInstance::InitializeExtraWindowUIContext(APlayerController* controller)
{
	// Creates the world context. This should be the only WorldContext that ever gets created for this GameInstance.
	WorldContext = &GetEngine()->CreateNewWorldContext(EWorldType::None);
	WorldContext->OwningGameInstance = this;
	//WorldContext->RunAsDedicated = true;

	// create a ExtraWindowUI world
	UWorld* ExtraWindowUIWorld = UWorld::CreateWorld(EWorldType::None, false, "ExtraWindowUI");

	ExtraWindowUIWorld->SetGameInstance(this);
	GEngine->DestroyWorldContext(ExtraWindowUIWorld);
	//WorldContext->SetCurrentWorld( ExtraWindowUIWorld );

	ExtraWindowUIWorld->AddController(controller);

	Init();
}

#if WITH_EDITOR
void UExtraWindowUIGameInstance::Shutdown()
{
	ExtraWindowUI->CloseExtraWindowUI();
	Super::Shutdown();
}
#endif