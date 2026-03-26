// Copyright Epic Games, Inc. All Rights Reserved.

#include "Panel/VSCalibratorPanel.h"

#include "PointCloud/VSCalibratorGeometryPostprocessUtils.h"
#include "Inference/VSCalibratorNNEHelper.h"
#include "PointCloud/VSCalibratorPointCloudUtils.h"

#include "DesktopPlatformModule.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "IDesktopPlatform.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "Widgets/SWindow.h"

namespace
{
	struct FVSCalibratorPointCloudOutputPaths
	{
		FString Directory;
		FString XyzPath;
	};

	FVSCalibratorPointCloudOutputPaths BuildPointCloudOutputPaths()
	{
		FVSCalibratorPointCloudOutputPaths Paths;
		Paths.Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("PointClouds"));
		IFileManager::Get().MakeDirectory(*Paths.Directory, true);

		const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
		Paths.XyzPath = FPaths::Combine(Paths.Directory, FString::Printf(TEXT("PointCloud_%s.xyz"), *Timestamp));
		return Paths;
	}
}

FReply SVSCalibratorPanel::OnCaptureImageClicked()
{
	SetPanelStatus(TEXT("Importing image"));

	FString FilePath;
	if (!PromptForOpenImageFile(FilePath))
	{
		SetPanelStatus(TEXT("Waiting for input"));
		SetRunSummaryLines({TEXT("Image import was cancelled.")});
		return FReply::Handled();
	}

	FString Error;
	if (!TryLoadCapturedFrameFromFile(FilePath, Error))
	{
		SetPanelStatus(TEXT("Import failed"));
		SetRunSummaryLines({FString::Printf(TEXT("Image import failed: %s"), *Error)});
		return FReply::Handled();
	}

	ResetDepthPreview();
	SetPanelStatus(TEXT("Image ready"));
	SetRunSummaryLines(
		{
			FString::Printf(TEXT("Loaded image: %s"), *CapturedFrame.ToSummaryString()),
			TEXT("Depth inference and point cloud export are ready to run.")
		});
	return FReply::Handled();
}

FReply SVSCalibratorPanel::OnGenerateOutputsClicked()
{
	if (ModelDirectoryTextBox.IsValid())
	{
		ModelDirectoryInput = ModelDirectoryTextBox->GetText().ToString().TrimStartAndEnd();
	}

	TArray<FString> SummaryLines;
	SetPanelStatus(TEXT("Running"));

	if (!bHasCapturedFrame)
	{
		SetPanelStatus(TEXT("Waiting for input"));
		SetRunSummaryLines({TEXT("Import an input image before running the pipeline.")});
		return FReply::Handled();
	}

	if (!bHasIntrinsics)
	{
		SetPanelStatus(TEXT("Configuration error"));
		SetRunSummaryLines({TEXT("Fixed camera intrinsics are not initialized.")});
		return FReply::Handled();
	}

	FString OnnxPath;
	FString ResolvedDirectory;
	FString ModelError;
	if (!ResolveModelFile(OnnxPath, ResolvedDirectory, ModelError))
	{
		SetPanelStatus(TEXT("Invalid model"));
		SetRunSummaryLines({FString::Printf(TEXT("Model directory validation failed: %s"), *ModelError)});
		return FReply::Handled();
	}

	SummaryLines.Add(FString::Printf(TEXT("Input image: %s"), *CapturedFrame.ToSummaryString()));
	SummaryLines.Add(FString::Printf(TEXT("Model directory: %s"), *ResolvedDirectory));
	SummaryLines.Add(FString::Printf(TEXT("Model file: %s"), *FPaths::GetCleanFilename(OnnxPath)));

	const double FocalPx = (Intrinsics.Fx + Intrinsics.Fy) * 0.5;
	const FVSCalibratorNNEDepthInferenceResult DepthResult =
		FVSCalibratorNNEHelper::RunDepthInference(OnnxPath, CapturedFrame, FocalPx);

	if (!DepthResult.bPassed)
	{
		SummaryLines.Add(FString::Printf(TEXT("Depth inference failed: %s"), *DepthResult.Details));
		SetPanelStatus(TEXT("Run failed"));
		SetRunSummaryLines(SummaryLines);
		return FReply::Handled();
	}

	UpdateDepthPreview(DepthResult.DepthMap);
	SummaryLines.Add(FString::Printf(TEXT("Inference time: %.2f ms"), DepthResult.InferenceMs));
	SummaryLines.Add(FString::Printf(TEXT("Depth result: %s"), *DepthResult.DepthMap.ToSummaryString()));

	TArray<FVector> PointCloud;
	FString PointCloudError;
	if (!VSCalibratorPointCloudUtils::BuildPointCloudFromDepth(DepthResult.DepthMap, Intrinsics, PointCloud, PointCloudError))
	{
		SummaryLines.Add(FString::Printf(TEXT("Point cloud generation failed: %s"), *PointCloudError));
		SetPanelStatus(TEXT("Run failed"));
		SetRunSummaryLines(SummaryLines);
		return FReply::Handled();
	}

	FVSCalibratorGeometryPostprocessConfig GeometryConfig = GeometryPostprocessConfig;
	if (GeometryConfig.bEnableManhattanConstraint && !GeometryConfig.bEnablePlaneDetection)
	{
		GeometryConfig.bEnableManhattanConstraint = false;
		SummaryLines.Add(TEXT("Geometry settings were normalized: Manhattan constraint requires plane detection."));
	}

	if (GeometryConfig.MaxSnapAngleDeg <= 0.0f)
	{
		GeometryConfig.MaxSnapAngleDeg = 8.0f;
		SummaryLines.Add(TEXT("Max snap angle was invalid and reset to 8.0."));
	}

	FVSCalibratorGeometryPostprocessResult GeometryResult;
	if (!VSCalibratorGeometryPostprocessUtils::RunPostDepthGeometry(PointCloud, GeometryConfig, GeometryResult, PointCloudError))
	{
		SummaryLines.Add(FString::Printf(TEXT("Geometry postprocess failed: %s"), *PointCloudError));
		SetPanelStatus(TEXT("Run failed"));
		SetRunSummaryLines(SummaryLines);
		return FReply::Handled();
	}

	if (GeometryConfig.bEnablePlaneDetection || GeometryConfig.bEnableManhattanConstraint)
	{
		SummaryLines.Add(FString::Printf(TEXT("Geometry stage: %s"), *GeometryResult.StageNote));
		if (!GeometryResult.Planes.IsEmpty())
		{
			SummaryLines.Add(FString::Printf(TEXT("Primary plane: %s"), *GeometryResult.Planes[0].ToSummaryString()));
		}
	}

	const TArray<FVector>& OutputPoints = GeometryResult.CameraPoints;
	SummaryLines.Add(FString::Printf(TEXT("Output point count: %d"), OutputPoints.Num()));

	const FVSCalibratorPointCloudOutputPaths OutputPaths = BuildPointCloudOutputPaths();
	if (!VSCalibratorPointCloudUtils::SavePointCloudAsXyz(OutputPaths.XyzPath, OutputPoints, PointCloudError))
	{
		SummaryLines.Add(FString::Printf(TEXT("XYZ export failed: %s"), *PointCloudError));
		SetPanelStatus(TEXT("Run failed"));
		SetRunSummaryLines(SummaryLines);
		return FReply::Handled();
	}

	SummaryLines.Add(FString::Printf(TEXT("XYZ export: %s"), *OutputPaths.XyzPath));
	SetPanelStatus(TEXT("Run completed"));
	SetRunSummaryLines(SummaryLines);
	return FReply::Handled();
}

bool SVSCalibratorPanel::PromptForOpenImageFile(FString& OutFilePath) const
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return false;
	}

	void* ParentWindowHandle = nullptr;
	if (FSlateApplication::IsInitialized())
	{
		const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindBestParentWindowForDialogs(nullptr);
		if (ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid())
		{
			ParentWindowHandle = ParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}
	}

	TArray<FString> OutFiles;
	const bool bOpened = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("Import Image"),
		FPaths::ProjectDir(),
		TEXT(""),
		TEXT("Image files (*.png;*.jpg;*.jpeg;*.bmp;*.exr)|*.png;*.jpg;*.jpeg;*.bmp;*.exr"),
		EFileDialogFlags::None,
		OutFiles);

	if (!bOpened || OutFiles.IsEmpty())
	{
		return false;
	}

	OutFilePath = OutFiles[0];
	return true;
}

bool SVSCalibratorPanel::TryLoadCapturedFrameFromFile(const FString& FilePath, FString& OutError)
{
	UTexture2D* LoadedTexture = FImageUtils::ImportFileAsTexture2D(FilePath);
	if (!LoadedTexture)
	{
		OutError = TEXT("Failed to decode image or the format is unsupported.");
		return false;
	}

	CapturedImageTexture.Reset(LoadedTexture);
	CapturedImageBrush.SetResourceObject(CapturedImageTexture.Get());
	CapturedImageBrush.ImageSize = FVector2D(
		static_cast<float>(CapturedImageTexture->GetSizeX()),
		static_cast<float>(CapturedImageTexture->GetSizeY()));

	CapturedFrame.SourcePath = FilePath;
	CapturedFrame.Width = CapturedImageTexture->GetSizeX();
	CapturedFrame.Height = CapturedImageTexture->GetSizeY();
	CapturedFrame.CaptureTimeUtc = FDateTime::UtcNow();
	bHasCapturedFrame = true;
	return true;
}

bool SVSCalibratorPanel::ResolveModelFile(FString& OutOnnxPath, FString& OutResolvedDirectory, FString& OutError) const
{
	OutOnnxPath.Reset();
	OutResolvedDirectory.Reset();

	FString Relative = ModelDirectoryInput.TrimStartAndEnd();
	if (Relative.IsEmpty())
	{
		OutError = TEXT("Model directory is empty.");
		return false;
	}

	Relative.ReplaceInline(TEXT("\\"), TEXT("/"));
	Relative.RemoveFromStart(TEXT("/Game/"));
	Relative.RemoveFromStart(TEXT("Game/"));
	Relative.RemoveFromStart(TEXT("/"));

	OutResolvedDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / Relative);
	if (!IFileManager::Get().DirectoryExists(*OutResolvedDirectory))
	{
		OutError = FString::Printf(TEXT("Directory does not exist: %s"), *OutResolvedDirectory);
		return false;
	}

	TArray<FString> OnnxFiles;
	IFileManager::Get().FindFiles(OnnxFiles, *(OutResolvedDirectory / TEXT("*.onnx")), true, false);
	if (OnnxFiles.IsEmpty())
	{
		OutError = TEXT("No .onnx file was found in the selected directory.");
		return false;
	}

	OnnxFiles.Sort();
	OutOnnxPath = OutResolvedDirectory / OnnxFiles[0];
	return true;
}

void SVSCalibratorPanel::ResetDepthPreview()
{
	bHasDepthPreview = false;
	DepthPreviewData = FVSCalibratorDepthMap();
	DepthImageTexture.Reset();
	DepthImageBrush.SetResourceObject(nullptr);
}

void SVSCalibratorPanel::UpdateDepthPreview(const FVSCalibratorDepthMap& DepthMap)
{
	ResetDepthPreview();

	if (!DepthMap.IsValid())
	{
		return;
	}

	const int32 Width = DepthMap.Width;
	const int32 Height = DepthMap.Height;
	if (Width <= 0 || Height <= 0)
	{
		return;
	}

	TArray<FColor> Pixels;
	Pixels.SetNumUninitialized(Width * Height);

	const float MinDepth = DepthMap.MinDepth;
	const float MaxDepth = DepthMap.MaxDepth;
	const float Range = MaxDepth - MinDepth;

	for (int32 Index = 0; Index < Pixels.Num(); ++Index)
	{
		float Normalized = 0.0f;
		if (Range > KINDA_SMALL_NUMBER)
		{
			Normalized = (DepthMap.Values[Index] - MinDepth) / Range;
		}

		Normalized = FMath::Clamp(Normalized, 0.0f, 1.0f);
		const uint8 Gray = static_cast<uint8>(Normalized * 255.0f);
		Pixels[Index] = FColor(Gray, Gray, Gray, 255);
	}

	UTexture2D* NewDepthTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!NewDepthTexture || !NewDepthTexture->GetPlatformData() || NewDepthTexture->GetPlatformData()->Mips.Num() == 0)
	{
		return;
	}

	void* MipData = NewDepthTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
	NewDepthTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
	NewDepthTexture->SRGB = false;
	NewDepthTexture->UpdateResource();

	DepthImageTexture.Reset(NewDepthTexture);
	DepthImageBrush.SetResourceObject(DepthImageTexture.Get());
	DepthImageBrush.ImageSize = FVector2D(static_cast<float>(Width), static_cast<float>(Height));

	DepthPreviewData = DepthMap;
	bHasDepthPreview = true;
}
