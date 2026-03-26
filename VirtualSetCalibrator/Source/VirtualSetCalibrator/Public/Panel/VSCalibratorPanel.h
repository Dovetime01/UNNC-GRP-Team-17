// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Core/VSCalibratorTypes.h"
#include "Widgets/SCompoundWidget.h"

class UTexture2D;
enum class ECheckBoxState : uint8;

class SVSCalibratorPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVSCalibratorPanel)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnCaptureImageClicked();
	FReply OnGenerateOutputsClicked();

	void SetPanelStatus(const FString& NewState);
	void SetRunSummaryLines(const TArray<FString>& Lines);
	void ResetDepthPreview();
	void UpdateDepthPreview(const FVSCalibratorDepthMap& DepthMap);

	bool PromptForOpenImageFile(FString& OutFilePath) const;
	bool TryLoadCapturedFrameFromFile(const FString& FilePath, FString& OutError);
	bool ResolveModelFile(FString& OutOnnxPath, FString& OutResolvedDirectory, FString& OutError) const;

	FText GetPanelStatusText() const;
	FText GetIntrinsicsSummaryText() const;
	FText GetCaptureSummaryText() const;
	FText GetDepthSummaryText() const;
	FText GetRunSummaryText() const;
	FText GetMaxSnapAngleText() const;

	ECheckBoxState GetEnablePlaneDetectionCheckState() const;
	void OnEnablePlaneDetectionCheckStateChanged(ECheckBoxState NewState);
	ECheckBoxState GetEnableManhattanCheckState() const;
	void OnEnableManhattanCheckStateChanged(ECheckBoxState NewState);
	void OnMaxSnapAngleCommitted(const FText& NewText, ETextCommit::Type CommitType);

	const FSlateBrush* GetCapturedImageBrush() const;
	const FSlateBrush* GetDepthImageBrush() const;

private:
	TSharedPtr<SEditableTextBox> ModelDirectoryTextBox;
	TSharedPtr<SEditableTextBox> MaxSnapAngleTextBox;

	FString CurrentStatus = TEXT("Waiting for input");
	FString CurrentRunSummary = TEXT("Import an image to run depth inference and export a point cloud.");
	FString ModelDirectoryInput = TEXT("NNModels/depth-anythingV3");

	FVSCalibratorCameraIntrinsics Intrinsics;
	bool bHasIntrinsics = false;

	FVSCalibratorFrameData CapturedFrame;
	bool bHasCapturedFrame = false;

	FVSCalibratorDepthMap DepthPreviewData;
	bool bHasDepthPreview = false;

	FVSCalibratorGeometryPostprocessConfig GeometryPostprocessConfig;

	FSlateBrush CapturedImageBrush;
	FSlateBrush DepthImageBrush;
	TStrongObjectPtr<UTexture2D> CapturedImageTexture;
	TStrongObjectPtr<UTexture2D> DepthImageTexture;
};
