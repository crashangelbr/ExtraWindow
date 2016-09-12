#pragma once

#include "Blueprint/UserWidget.h"
#include "Engine/GameEngine.h"
#include "SGameLayerManager.h"
#include "Slate/SceneViewport.h"
#include "Runtime/Slate/Public/Widgets/Layout/SConstraintCanvas.h"
#include "ExtraWindowGameInstance.h"
#include "ExtraWindowUI.generated.h"

UCLASS()
class UExtraWindowUI : public UUserWidget
{
	GENERATED_BODY()

		UExtraWindowUI(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExtraWindowUI")
		bool bExtraWindowUICreated;

	UFUNCTION(BlueprintCallable, Category = "ExtraWindowUI")
		bool CreateExtraWindowUI();

	UFUNCTION(BlueprintCallable, Category = "ExtraWindowUI")
		void UpdateExtraWindowUI(float DeltaSeconds);

	UFUNCTION(BlueprintPure, Category = "ExtraWindowUI")
		FText GetFloatAsTextWithPrecision(float inFloat);

	UFUNCTION(BlueprintPure, Category = "ExtraWindowUI")
		static FVector2D GetWINCursorPosition();

	UFUNCTION(BlueprintPure, Category = "ExtraWindowUI")
		static bool GetWINLeftMouseButtonDown();

	UFUNCTION(BlueprintCallable, Category = "Extra Window|UI")
		static void GetResolutionMonitor(bool isSecondMonitor, FVector2D& MonitorPosition, FVector2D& MonitorResolution);

	void CloseExtraWindowUI();

private:

	UGameViewportClient* ExtraWindowUIViewportClient;
	TSharedPtr<FSceneViewport> ExtraWindowUISceneViewport;
	FNumberFormattingOptions NumberFormat;
	FVector2D GameWindowLocation;
	FVector2D GameWindowSize;

	void OnExtraWindowUIClosed(const TSharedRef<SWindow>& WindowBeingClosed);
};
