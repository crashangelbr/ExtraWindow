// Fill out your copyright notice in the Description page of Project Settings.

#include "IExtraWindowPlugin.h"
#include "ExtraWindowGameManager.h"
#include "EngineModule.h"
#include "ExtraWindowUI.h"

// Sets default values
AExtraWindowGameManager::AExtraWindowGameManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bGameWindowShown = true;
}

// Called when the game starts or when spawned
void AExtraWindowGameManager::BeginPlay()
{
	Super::BeginPlay();

	UGameEngine* gameEngine = Cast<UGameEngine>(GEngine);
	if (gameEngine != NULL)
	{
		GameWindow = gameEngine->GameViewportWindow;
		GameWindow.Pin()->SetOnWindowClosed(FOnWindowClosed::CreateUObject(this, &AExtraWindowGameManager::OnGameWindowClosed));
		GEngine->GetFirstLocalPlayerController(GetWorld())->bShowMouseCursor = true;
		//gameEngine->GameViewport->SetCaptureMouseOnClick( EMouseCaptureMode::NoCapture );
		//gameEngine->GameViewport->GetWindow()->GetNativeWindow()->AdjustCachedSize( FVector2D( 100, 100 ) );
	}
}

// Called every frame
void AExtraWindowGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRunning)
	{
		ShowGameWindow();
	}
	else
	{
		HideGameWindow();
	}

}


void AExtraWindowGameManager::ShowGameWindow()
{
	if (GameWindow.IsValid())
	{
		UExtraWindowUI::GetResolutionMonitor(true, GameWindowLocation, GameWindowSize);

		if (!bGameWindowShown)
		{
			GameWindowHiddenWidget->RemoveFromViewport();

			SetGraphicsQuality(GraphicsQuality);

			GameWindow.Pin()->ReshapeWindow(GameWindowLocation, GameWindowSize);
			FSystemResolution::RequestResolutionChange(GameWindowSize.X, GameWindowSize.Y, EWindowMode::WindowedFullscreen);

			GEngine->GetFirstLocalPlayerController(GetWorld())->SetPause(false);

			bGameWindowShown = true;
		}

		//do the reshape if neccesary
		if (GameWindow.Pin()->GetSizeInScreen().X != GameWindowSize.X)
		{
			GameWindow.Pin()->ReshapeWindow(GameWindowLocation, GameWindowSize);
		}
	}
}

void AExtraWindowGameManager::HideGameWindow()
{

	if (GameWindow.IsValid())
	{
		// if the game-window is in shown-mode, set it in hidden-mode
		if (bGameWindowShown)
		{
			GameWindowHiddenWidget->AddToViewport(0);

			SetGraphicsQuality(0);

			FSystemResolution::RequestResolutionChange(100, 100, EWindowMode::WindowedFullscreen);

			GEngine->GetFirstLocalPlayerController(GetWorld())->SetPause(true);

			bGameWindowShown = false;
		}

		//set reshape of the game window if size not proper to hidden-mode
		bool bReshapeWindow = false;
		if (GameWindow.Pin()->GetSizeInScreen().X != 150)
		{
			GameWindowSize.X = 150;
			GameWindowSize.Y = 150;
			bReshapeWindow = true;
		}

		//set relocation of the game window if dragged
		if (bGameWindowDragged)
		{
			if (UExtraWindowUI::GetWINLeftMouseButtonDown())
			{
				GameWindowLocation = UExtraWindowUI::GetWINCursorPosition() - 75;
			}
			else
			{
				int32 x, y, w, h = 0;
				GameWindow.Pin()->GetNativeWindow()->GetFullScreenInfo(x, y, w, h);
				GameWindowLocation.X = x;
				GameWindowLocation.Y = y;
				bGameWindowDragged = false;
			}
			bReshapeWindow = true;
		}

		//do the reshape (and relocation) if neccesary
		if (bReshapeWindow)
		{
			GameWindow.Pin()->ReshapeWindow(GameWindowLocation, GameWindowSize);
		}

	}
}

void AExtraWindowGameManager::OnGameWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed)
{
	ExtraWindowUI->CloseExtraWindowUI();
	StopGame();
}

void AExtraWindowGameManager::SetGraphicsQuality(const int32 newGraphicsQuality)
{
	GEngine->GetGameUserSettings()->ScalabilityQuality.SetFromSingleQualityLevel(newGraphicsQuality);
	GEngine->GetGameUserSettings()->ApplyNonResolutionSettings();
}