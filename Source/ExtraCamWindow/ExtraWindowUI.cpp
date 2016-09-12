#include "IExtraWindowPlugin.h"
#include "ExtraWindowUI.h"

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

UExtraWindowUI::UExtraWindowUI(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if WITH_EDITOR
	isEditor = true;
#endif

	//set Float precision for displaying values
	NumberFormat.MinimumIntegralDigits = 1;
	NumberFormat.MaximumIntegralDigits = 11111111;
	NumberFormat.MinimumFractionalDigits = 3;
	NumberFormat.MaximumFractionalDigits = 3;
}

void UExtraWindowUI::CloseExtraWindowUI() {
	if (ExtraWindowViewportClient != nullptr) {
		ExtraWindowViewportClient->GetWindow()->DestroyWindowImmediately();
		
		if (!isEditor) {
			GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
		}
	}
}

bool UExtraWindowUI::CreateExtraWindowUI()
{
	bExtraWindowCreated = false;

	if (GIsClient)
	{
		//Initialize GameInstance of new viewport client
		UExtraWindowUIGameInstance* ExtraWindowGameInstance = NewObject<UExtraWindowUIGameInstance>(GEngine, UExtraWindowUIGameInstance::StaticClass());
		ExtraWindowGameInstance->InitializeExtraWindowUIContext(GetWorld()->GetFirstPlayerController());

		//Initialize Viewport Client
		ExtraWindowViewportClient = NewObject<UGameViewportClient>(GEngine, UGameViewportClient::StaticClass());
		ExtraWindowViewportClient->Init(*ExtraWindowGameInstance->GetWorldContext(), ExtraWindowGameInstance);

		ExtraWindowGameInstance->GetWorldContext()->GameViewport = ExtraWindowViewportClient;

		if (ExtraWindowViewportClient)
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
			Window->SetOnWindowClosed(FOnWindowClosed::CreateUObject(this, &UExtraWindowUI::OnExtraWindowWindowClosed));

			// Attach the viewport client to a new viewport.
			TSharedRef<SOverlay> ViewportOverlayWidgetRef = SNew(SOverlay);
			ViewportOverlayWidgetRef->SetCursor(EMouseCursor::Default);

			TSharedRef<SGameLayerManager> LayerManagerRef = SNew(SGameLayerManager)
				.SceneViewport(ExtraWindowViewportClient->GetGameViewport())
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

			ExtraWindowViewportClient->SetViewportOverlayWidget(Window, ViewportOverlayWidgetRef);
			ExtraWindowViewportClient->SetGameLayerManager(LayerManagerRef);

			ExtraWindowSceneViewport = MakeShareable(new FSceneViewport(ExtraWindowViewportClient, Viewport));
			ExtraWindowViewportClient->Viewport = ExtraWindowSceneViewport.Get();
			Viewport->SetViewportInterface(ExtraWindowSceneViewport.ToSharedRef());

			FViewportFrame* ViewportFrame = ExtraWindowSceneViewport.Get();
			ExtraWindowViewportClient->SetViewportFrame(ViewportFrame);

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
			ExtraWindowViewportClient->AddViewportWidgetContent(ViewportWidget, 10);

			ExtraWindowViewportClient->SetCaptureMouseOnClick(EMouseCaptureMode::NoCapture);

			//ExtraWindowViewportClient->bDisableWorldRendering = true;
			//ExtraWindowSceneViewport->SetGameRenderingEnabled( false );
			//ExtraWindowSceneViewport->ProcessToggleFreezeCommand();
			//FSlateApplication::Get().ReleaseMouseCapture();

			Window->ShowWindow();
		}
	}

	return bExtraWindowCreated;
}

void UExtraWindowUI::OnExtraWindowWindowClosed(const TSharedRef<SWindow>& WindowBeingClosed)
{
	ExtraWindowViewportClient->CloseRequested(ExtraWindowSceneViewport->GetViewport());
	ExtraWindowSceneViewport.Reset();

	CloseExtraWindowUI();

	ExtraWindowViewportClient = NULL;
	ExtraWindowSceneViewport = NULL;

	if (isEditor) {
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
	}

	UGameEngine* gameEngine = Cast<UGameEngine>(GEngine);
	if (gameEngine)
	{
		if (gameEngine->GameViewportWindow.IsValid())
		{
			gameEngine->OnGameWindowClosed(gameEngine->GameViewportWindow.Pin().ToSharedRef());
		}
	}

	bExtraWindowCreated = false;
}

void UExtraWindowUI::UpdateExtraWindowUI(float DeltaSeconds)
{
	if (ExtraWindowSceneViewport->HasMouseCapture())
		FSlateApplication::Get().ReleaseMouseCapture();


	if (!ExtraWindowViewportClient->GetWindow()->IsViewportSizeDrivenByWindow())
		ExtraWindowViewportClient->GetWindow()->SetViewportSizeDrivenByWindow(true);
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