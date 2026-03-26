// Copyright Epic Games, Inc. All Rights Reserved.

#include "Panel/VSCalibratorPanel.h"

#include "Styling/CoreStyle.h"

void SVSCalibratorPanel::SetPanelStatus(const FString& NewState)
{
	CurrentStatus = NewState;
}

void SVSCalibratorPanel::SetRunSummaryLines(const TArray<FString>& Lines)
{
	if (Lines.IsEmpty())
	{
		CurrentRunSummary = TEXT("Import an image to run depth inference and export a point cloud.");
		return;
	}

	CurrentRunSummary = FString::Join(Lines, TEXT("\n"));
}

FText SVSCalibratorPanel::GetPanelStatusText() const
{
	return FText::FromString(FString::Printf(TEXT("Status: %s"), *CurrentStatus));
}

FText SVSCalibratorPanel::GetIntrinsicsSummaryText() const
{
	if (!bHasIntrinsics)
	{
		return FText::FromString(TEXT("Camera intrinsics are not initialized."));
	}

	const double FocalPx = (Intrinsics.Fx + Intrinsics.Fy) * 0.5;
	return FText::FromString(
		FString::Printf(TEXT("Fixed intrinsics: %s | focal_px=%.3f"), *Intrinsics.ToSummaryString(), FocalPx));
}

FText SVSCalibratorPanel::GetCaptureSummaryText() const
{
	if (!bHasCapturedFrame)
	{
		return FText::FromString(TEXT("No input image has been imported."));
	}

	return FText::FromString(FString::Printf(TEXT("Current image: %s"), *CapturedFrame.ToSummaryString()));
}

FText SVSCalibratorPanel::GetDepthSummaryText() const
{
	if (!bHasDepthPreview)
	{
		return FText::FromString(TEXT("No depth preview is available yet."));
	}

	return FText::FromString(FString::Printf(TEXT("Depth result: %s"), *DepthPreviewData.ToSummaryString()));
}

FText SVSCalibratorPanel::GetRunSummaryText() const
{
	return FText::FromString(CurrentRunSummary);
}

FText SVSCalibratorPanel::GetMaxSnapAngleText() const
{
	return FText::FromString(FString::Printf(TEXT("%.2f"), GeometryPostprocessConfig.MaxSnapAngleDeg));
}

ECheckBoxState SVSCalibratorPanel::GetEnablePlaneDetectionCheckState() const
{
	return GeometryPostprocessConfig.bEnablePlaneDetection ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SVSCalibratorPanel::OnEnablePlaneDetectionCheckStateChanged(ECheckBoxState NewState)
{
	GeometryPostprocessConfig.bEnablePlaneDetection = (NewState == ECheckBoxState::Checked);
	if (!GeometryPostprocessConfig.bEnablePlaneDetection)
	{
		GeometryPostprocessConfig.bEnableManhattanConstraint = false;
	}
}

ECheckBoxState SVSCalibratorPanel::GetEnableManhattanCheckState() const
{
	return GeometryPostprocessConfig.bEnableManhattanConstraint ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SVSCalibratorPanel::OnEnableManhattanCheckStateChanged(ECheckBoxState NewState)
{
	const bool bEnable = (NewState == ECheckBoxState::Checked);
	if (bEnable && !GeometryPostprocessConfig.bEnablePlaneDetection)
	{
		GeometryPostprocessConfig.bEnableManhattanConstraint = false;
		SetRunSummaryLines({TEXT("Enable plane detection before enabling the Manhattan constraint.")});
		return;
	}

	GeometryPostprocessConfig.bEnableManhattanConstraint = bEnable;
}

void SVSCalibratorPanel::OnMaxSnapAngleCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnCleared)
	{
		return;
	}

	const FString Trimmed = NewText.ToString().TrimStartAndEnd();
	if (Trimmed.IsEmpty())
	{
		if (MaxSnapAngleTextBox.IsValid())
		{
			MaxSnapAngleTextBox->SetText(GetMaxSnapAngleText());
		}
		return;
	}

	const float ParsedValue = FCString::Atof(*Trimmed);
	if (!FMath::IsFinite(ParsedValue) || ParsedValue <= 0.0f || ParsedValue > 45.0f)
	{
		if (MaxSnapAngleTextBox.IsValid())
		{
			MaxSnapAngleTextBox->SetText(GetMaxSnapAngleText());
		}
		SetRunSummaryLines({TEXT("Max snap angle must stay within the (0, 45] range.")});
		return;
	}

	GeometryPostprocessConfig.MaxSnapAngleDeg = ParsedValue;
	if (MaxSnapAngleTextBox.IsValid())
	{
		MaxSnapAngleTextBox->SetText(GetMaxSnapAngleText());
	}
}

const FSlateBrush* SVSCalibratorPanel::GetCapturedImageBrush() const
{
	if (!bHasCapturedFrame || !CapturedImageTexture.IsValid())
	{
		return FStyleDefaults::GetNoBrush();
	}

	return &CapturedImageBrush;
}

const FSlateBrush* SVSCalibratorPanel::GetDepthImageBrush() const
{
	if (!bHasDepthPreview || !DepthImageTexture.IsValid())
	{
		return FStyleDefaults::GetNoBrush();
	}

	return &DepthImageBrush;
}
