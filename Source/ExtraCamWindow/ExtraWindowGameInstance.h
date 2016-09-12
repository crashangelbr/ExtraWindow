// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "ExtraWindowGameManager.h"
#include "ExtraWindowGameInstance.generated.h"

class UExtraWindowUI;

UCLASS()
class UExtraWindowUIGameInstance : public UGameInstance
{
	GENERATED_BODY()

protected:

#if WITH_EDITOR
		virtual void Shutdown() override;
#endif

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager")
		UExtraWindowUI* ExtraWindowUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager")
		AExtraWindowGameManager* ExtraWindowGameManager;

	void InitializeExtraWindowUIContext(APlayerController* controller);

};