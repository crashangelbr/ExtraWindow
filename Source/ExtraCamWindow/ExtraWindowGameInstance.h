#pragma once

#include "Engine/GameInstance.h"
#include "ExtraWindowGameManager.h"
#include "ExtraWindowGameInstance.generated.h"

class UExtraWindowUI;

UCLASS()
class UExtraWindowGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extra Window|Game Instance")
		UExtraWindowUI* ExtraWindowUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extra Window|Game Instance")
		AExtraWindowGameManager* GameManager;

	UFUNCTION(BlueprintCallable, BlueprintCallable, Category = "Extra Window|Game Instance")
		void InitializeExtraWindowContext(APlayerController* controller);
};
