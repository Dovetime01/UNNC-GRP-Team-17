// Copyright Epic Games, Inc. All Rights Reserved.

#include "Panel/VSCalibratorPanel.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SVSCalibratorPanel"

void SVSCalibratorPanel::Construct(const FArguments& InArgs)
{
	CapturedImageBrush.DrawAs = ESlateBrushDrawType::Image;
	DepthImageBrush.DrawAs = ESlateBrushDrawType::Image;

	Intrinsics.ImageWidth = 4032;
	Intrinsics.ImageHeight = 3024;
	Intrinsics.Fx = 2688.0;
	Intrinsics.Fy = 2688.0;
	Intrinsics.Cx = 2016.0;
	Intrinsics.Cy = 1512.0;
	Intrinsics.Distortion.Reset();
	bHasIntrinsics = true;

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 10.0f, 10.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PanelTitle", "Virtual Set Calibrator"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 8.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(this, &SVSCalibratorPanel::GetPanelStatusText)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f)
			[
				SNew(SSeparator)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 10.0f, 10.0f, 8.0f)
			[
				SNew(SBorder)
				.Padding(8.0f)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT(
						"FixedCameraReminder",
						"This panel is focused on a fixed iPhone 14 Pro 1x setup, depth inference, and camera-space point cloud export."))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 8.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(this, &SVSCalibratorPanel::GetIntrinsicsSummaryText)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 4.0f, 10.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ModelDirectoryLabel", "Model Directory (Relative to Content)"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 8.0f)
			[
				SAssignNew(ModelDirectoryTextBox, SEditableTextBox)
				.Text(FText::FromString(ModelDirectoryInput))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GeometryTitle", "Geometry Postprocess"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 4.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SVSCalibratorPanel::GetEnablePlaneDetectionCheckState)
				.OnCheckStateChanged(this, &SVSCalibratorPanel::OnEnablePlaneDetectionCheckStateChanged)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("EnablePlaneDetection", "Enable Plane Detection"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 4.0f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SVSCalibratorPanel::GetEnableManhattanCheckState)
				.OnCheckStateChanged(this, &SVSCalibratorPanel::OnEnableManhattanCheckStateChanged)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("EnableManhattan", "Enable Manhattan Constraint"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 10.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MaxSnapAngleLabel", "Max Snap Angle"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(MaxSnapAngleTextBox, SEditableTextBox)
					.Text(GetMaxSnapAngleText())
					.OnTextCommitted(this, &SVSCalibratorPanel::OnMaxSnapAngleCommitted)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 10.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0.0f, 0.0f, 5.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("Capture", "Import Image"))
					.OnClicked(this, &SVSCalibratorPanel::OnCaptureImageClicked)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(5.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("GenerateOutputs", "Generate Depth + Point Cloud"))
					.OnClicked(this, &SVSCalibratorPanel::OnGenerateOutputsClicked)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f)
			[
				SNew(SSeparator)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 10.0f, 10.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CapturePreviewTitle", "Input Image"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 6.0f)
			[
				SNew(SBorder)
				.Padding(8.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(320.0f)
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFit)
							.StretchDirection(EStretchDirection::Both)
							[
								SNew(SImage)
								.Image(this, &SVSCalibratorPanel::GetCapturedImageBrush)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 8.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(this, &SVSCalibratorPanel::GetCaptureSummaryText)
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 6.0f, 10.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DepthPreviewTitle", "Depth Preview"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 6.0f)
			[
				SNew(SBorder)
				.Padding(8.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(320.0f)
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFit)
							.StretchDirection(EStretchDirection::Both)
							[
								SNew(SImage)
								.Image(this, &SVSCalibratorPanel::GetDepthImageBrush)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 8.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(this, &SVSCalibratorPanel::GetDepthSummaryText)
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 6.0f, 10.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RunSummaryTitle", "Run Summary"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(10.0f, 0.0f, 10.0f, 10.0f)
			[
				SNew(SBorder)
				.Padding(8.0f)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(this, &SVSCalibratorPanel::GetRunSummaryText)
				]
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
