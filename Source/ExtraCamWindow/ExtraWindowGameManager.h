#pragma once

#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "ExtraWindowGameManager.generated.h"

UCLASS()
class AExtraWindowGameManager : public AActor
{
	GENERATED_BODY()

public:

	AExtraWindowGameManager();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extra Window|Game Manager")
		bool bRunning;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Extra Window|Game Manager")
		float StopGame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extra Window|Game Manager")
		int32 GraphicsQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extra Window|Game Manager|Window")
		UUserWidget* GameWindowHiddenWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extra Window|Game Manager|Window")
		bool bGameWindowShown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extra Window|Game Manager|Window")
		bool bGameWindowDragged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extra Window|Game Manager|Window")
		FVector2D GameWindowDragOffset;

	UFUNCTION(BlueprintCallable, Category = "Extra Window|Game Manager|Window")
		void ShowGameWindow();

	UFUNCTION(BlueprintCallable, Category = "Extra Window|Game Manager|Window")
		void HideGameWindow();

	UFUNCTION(BlueprintCallable, Category = "Extra Window|Game Manager")
		void SetGraphicsQuality(const int32 newGraphicsQuality);

private:

	TWeakPtr<SWindow> GameWindow;

	FVector2D GameWindowLocation;
	FVector2D GameWindowSize;

	void OnGameWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed);
};