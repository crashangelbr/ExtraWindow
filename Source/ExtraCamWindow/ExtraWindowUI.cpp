#include "IExtraWindowPlugin.h"
#include "ExtraWindowUI.h"
#include "ExtraWindowGameInstance.h"

UExtraWindowUI::UExtraWindowUI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NumberFormat.MinimumIntegralDigits = 1;
	NumberFormat.MaximumIntegralDigits = 11111111;
	NumberFormat.MinimumFractionalDigits = 3;
	NumberFormat.MaximumFractionalDigits = 3;
}

void UExtraWindowUI::GetResolutionMonitor(bool isNoMainViewport, FVector2D& Position, FVector2D& Resolution)
{
	float isEditor = false;

#if WITH_EDITOR
	isEditor = true;
#endif

	int32 MonitorIndex = 1;

	if (!isNoMainViewport) { MonitorIndex = 0; }

	FDisplayMetrics Display;
	FDisplayMetrics::GetDisplayMetrics(Display);

	int32 PrimaryScreenX = Display.PrimaryDisplayWidth;
	int32 PrimaryScreenY = Display.PrimaryDisplayHeight;

	Position = FVector2D(0, 0);
	Resolution = FVector2D(PrimaryScreenX, PrimaryScreenY);

	if (!isEditor)
	{
		if (isNoMainViewport) { MonitorIndex = 1; }
		else { MonitorIndex = 0; }

		Position = FVector2D(-Display.MonitorInfo[MonitorIndex].NativeWidth, 0);
		Resolution = FVector2D(Display.MonitorInfo[MonitorIndex].NativeWidth, Display.MonitorInfo[MonitorIndex].NativeHeight);
	}
	else  if (isEditor && !isNoMainViewport) {
		if (isNoMainViewport) { MonitorIndex = 0; }
		else { MonitorIndex = 1; }

		Position = FVector2D(-Display.MonitorInfo[MonitorIndex].NativeWidth, 0);
		Resolution = FVector2D(Display.MonitorInfo[MonitorIndex].NativeWidth, Display.MonitorInfo[MonitorIndex].NativeHeight);
	}
}

bool UExtraWindowUI::CreateExtraWindowUIWindow()
{
	bExtraWindowUICreated = false;

	if (GIsClient)
	{
		//Initialize GameInstance of new viewport client
		UExtraWindowGameInstance* ExtraWindowUIGameInstance = NewObject<UExtraWindowGameInstance>(GEngine, UExtraWindowGameInstance::StaticClass());
		ExtraWindowUIGameInstance->InitializeExtraWindowContext(GetWorld()->GetFirstPlayerController());

		//Initialize Viewport Client
		ExtraWindowUIViewportClient = NewObject<UGameViewportClient>(GEngine, UGameViewportClient::StaticClass());
		ExtraWindowUIViewportClient->Init(*ExtraWindowUIGameInstance->GetWorldContext(), ExtraWindowUIGameInstance);

		ExtraWindowUIGameInstance->GetWorldContext()->GameViewport = ExtraWindowUIViewportClient;

		if (ExtraWindowUIViewportClient)
		{
			UExtraWindowUI::GetResolutionMonitor(false, GameWindowLocation, GameWindowSize);

			//create and register a new Window
			TSharedRef<SWindow> Window = SNew(SWindow)
				.ScreenPosition(GameWindowLocation)
				.ClientSize(GameWindowSize)
				.SizingRule(ESizingRule::UserSized)
				.Title(FText::FromString(TEXT("ExtraWindowUI")))
				.AutoCenter(EAutoCenter::None)
				.FocusWhenFirstShown(true)
				.UseOSWindowBorder(false)
				.CreateTitleBar(false)
				//.SupportsTransparency(EWindowTransparency::PerWindow)
				//.InitialOpacity(0.99f)
				.SupportsMaximize(true)
				.IsInitiallyMaximized(true)
				//.IsTopmostWindow(true)
				;

			FSlateApplication::Get().AddWindow(Window, false);
			Window->SetViewportSizeDrivenByWindow(false);
			Window->SetOnWindowClosed(FOnWindowClosed::CreateUObject(this, &UExtraWindowUI::OnExtraWindowUIWindowClosed));

			// Attach the viewport client to a new viewport.
			TSharedRef<SOverlay> ViewportOverlayWidgetRef = SNew(SOverlay);
			ViewportOverlayWidgetRef->SetCursor(EMouseCursor::Default);

			TSharedRef<SGameLayerManager> LayerManagerRef = SNew(SGameLayerManager)
				.SceneViewport(ExtraWindowUIViewportClient->GetGameViewport())
				.Visibility(EVisibility::Visible)
				.UseScissor(true)
				[
					ViewportOverlayWidgetRef
				];

			TSharedPtr<class SViewport> Viewport = SNew(SViewport)
				.RenderDirectlyToWindow(true)
				.EnableStereoRendering(true)
				[
					LayerManagerRef
				];

			ExtraWindowUIViewportClient->SetViewportOverlayWidget(Window, ViewportOverlayWidgetRef);
			ExtraWindowUIViewportClient->SetGameLayerManager(LayerManagerRef);

			ExtraWindowUISceneViewport = MakeShareable(new FSceneViewport(ExtraWindowUIViewportClient, Viewport));
			ExtraWindowUIViewportClient->Viewport = ExtraWindowUISceneViewport.Get();
			Viewport->SetViewportInterface(ExtraWindowUISceneViewport.ToSharedRef());

			FViewportFrame* ViewportFrame = ExtraWindowUISceneViewport.Get();
			ExtraWindowUIViewportClient->SetViewportFrame(ViewportFrame);

			Window->SetContent(Viewport.ToSharedRef());

			TSharedPtr<SWidget> UserSlateWidget = TakeWidget();
			TSharedRef<SConstraintCanvas> ViewportWidget = SNew(SConstraintCanvas);
			ViewportWidget->AddSlot()
				.Offset(BIND_UOBJECT_ATTRIBUTE(FMargin, GetFullScreenOffset))
				.Anchors(BIND_UOBJECT_ATTRIBUTE(FAnchors, GetViewportAnchors))
				.Alignment(BIND_UOBJECT_ATTRIBUTE(FVector2D, GetFullScreenAlignment))
				[
					UserSlateWidget.ToSharedRef()
				];

			ExtraWindowUIViewportClient->AddViewportWidgetContent(ViewportWidget, 10);

			ExtraWindowUIViewportClient->SetCaptureMouseOnClick(EMouseCaptureMode::NoCapture);

			//ExtraWindowUIViewportClient->bDisableWorldRendering = true;
			//ExtraWindowUISceneViewport->SetGameRenderingEnabled( false );
			//ExtraWindowUISceneViewport->ProcessToggleFreezeCommand();
			//FSlateApplication::Get().ReleaseMouseCapture();

			Window->ShowWindow();
		}
	}

	return bExtraWindowUICreated;
}

void UExtraWindowUI::OnExtraWindowUIWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed)
{

	ExtraWindowUIViewportClient->CloseRequested(ExtraWindowUISceneViewport->GetViewport());
	ExtraWindowUISceneViewport.Reset();

	ExtraWindowUIViewportClient = NULL;
	ExtraWindowUISceneViewport = NULL;

	GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");

	UGameEngine* gameEngine = Cast<UGameEngine>(GEngine);
	if (gameEngine)
	{
		if (gameEngine->GameViewportWindow.IsValid())
		{
			gameEngine->OnGameWindowClosed(gameEngine->GameViewportWindow.Pin().ToSharedRef());
		}
	}

	bExtraWindowUICreated = false;
}

void UExtraWindowUI::UpdateExtraWindowUI(float DeltaSeconds)
{
	if (ExtraWindowUISceneViewport->HasMouseCapture())
		FSlateApplication::Get().ReleaseMouseCapture();


	if (!ExtraWindowUIViewportClient->GetWindow()->IsViewportSizeDrivenByWindow())
		ExtraWindowUIViewportClient->GetWindow()->SetViewportSizeDrivenByWindow(true);
}

FText UExtraWindowUI::GetFloatAsTextWithPrecision(float inFloat)
{
	return FText::AsNumber(inFloat, &NumberFormat);
}

FVector2D UExtraWindowUI::GetWINCursorPosition()
{
	POINT cursorPosWIN;
	GetCursorPos(&cursorPosWIN);

	FVector2D cursorPosUE4(cursorPosWIN.x, cursorPosWIN.y);
	return cursorPosUE4;
}

bool UExtraWindowUI::GetWINLeftMouseButtonDown()
{
	SHORT leftMouseButtonState;
	leftMouseButtonState = GetKeyState(VK_LBUTTON);

	return (leftMouseButtonState & 0x80) > 0;
}