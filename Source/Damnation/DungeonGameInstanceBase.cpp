// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGameInstanceBase.h"
#include "Widgets/Layout/SConstraintCanvas.h"

/** Loading widget used between levels */
class SDungeonLoadingScreenWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDungeonLoadingScreenWidget)
		: _LoadingRotator(nullptr),
		  _LoadingSymbolRotateTime(1.0f)
	{}
	SLATE_ARGUMENT(UTexture2D*, LoadingRotator)
	SLATE_ARGUMENT(UTexture2D*, LoadingBG)
	SLATE_ARGUMENT(float, LoadingImageEdgeSize)
	SLATE_ARGUMENT(float, LoadingSymbolRotateTime)
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
	{
		LoadingBrush.SetResourceObject(InArgs._LoadingRotator);
		LoadingBrush.ImageSize.X = InArgs._LoadingImageEdgeSize;
		LoadingBrush.ImageSize.Y = InArgs._LoadingImageEdgeSize;
		LoadingBrush.DrawAs = ESlateBrushDrawType::Image;

		LoadingBG.SetResourceObject(InArgs._LoadingBG);
		LoadingBG.ImageSize.X = InArgs._LoadingBG->GetSurfaceWidth();
		LoadingBG.ImageSize.Y = InArgs._LoadingBG->GetSurfaceHeight();
		LoadingBG.DrawAs = ESlateBrushDrawType::Image;

		RotationSpeed = InArgs._LoadingSymbolRotateTime;

		// SWidget::RegisterActiveTimer(0, FWidgetActiveTimerDelegate::CreateSP(this, &SDungeonLoadingScreenWidget::LoadRotatorTick));
		ChildSlot
			[
			SNew(SConstraintCanvas)
			+ SConstraintCanvas::Slot()
				.Anchors(FAnchors(1, 1, 1, 1))
				.Offset(FMargin(-200, -200))
				.AutoSize(true)
				.ZOrder(1)
				[
					SNew(SSpinningImage)
					.Image(&LoadingBrush)
					.Period(RotationSpeed)
				]
			+ SConstraintCanvas::Slot()
				.Anchors(FAnchors(0, 0, 1, 1))
				.ZOrder(0)
				[
					SNew(SImage)
					.Image(&LoadingBG)
				]
			];
	}

protected:
	FSlateBrush LoadingBrush;
	FSlateBrush LoadingBG;
	SCanvas* LoadingCanvas;

	float CurrentRotation = 0.0f;
	float RotationSpeed = 1.0f;

private:
	EVisibility GetLoadIndicatorVisibility() const
	{
		return GetMoviePlayer()->IsLoadingFinished() ? EVisibility::Collapsed : EVisibility::Visible;
	}

	EVisibility GetMessageIndicatorVisibility() const
	{
		return GetMoviePlayer()->IsLoadingFinished() ? EVisibility::Visible : EVisibility::Collapsed;
	}

	//EActiveTimerReturnType LoadRotatorTick(double InCurrentTime, float InDeltaTime)
	//{
	//	CurrentRotation = FMath::Fmod(CurrentRotation + (InDeltaTime * RotationSpeed), 360.0f);
	//	SCanvas::SetRenderTransform(FSlateRenderTransform());
	//	SWidget::SetRenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(CurrentRotation))));
	//	return EActiveTimerReturnType::Continue;
	//}
};

void UDungeonGameInstanceBase::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UDungeonGameInstanceBase::BeginLoadingScreen);
 	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UDungeonGameInstanceBase::EndLoadingScreen);
}

void UDungeonGameInstanceBase::BeginLoadingScreen(const FString& InMapName)
{
	if (!IsRunningDedicatedServer())
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
		// LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
		LoadingScreen.WidgetLoadingScreen = SNew(SDungeonLoadingScreenWidget)
			.LoadingRotator(LoadingScreenImage)
			.LoadingImageEdgeSize(128)
			.LoadingSymbolRotateTime(LoadingScreenIconRotateSpeed)
			.LoadingBG(LoadingScreenBackground);

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
}

void UDungeonGameInstanceBase::EndLoadingScreen(UWorld* InLoadedWorld)
{

}
