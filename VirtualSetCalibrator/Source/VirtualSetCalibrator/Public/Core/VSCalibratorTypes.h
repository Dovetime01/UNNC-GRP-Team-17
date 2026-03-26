// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"

struct FVSCalibratorCameraIntrinsics
{
	int32 ImageWidth = 0;
	int32 ImageHeight = 0;

	double Fx = 0.0;
	double Fy = 0.0;
	double Cx = 0.0;
	double Cy = 0.0;

	TArray<double> Distortion;

	bool IsValid(FString* OutError = nullptr) const
	{
		if (ImageWidth <= 0 || ImageHeight <= 0)
		{
			if (OutError)
			{
				*OutError = TEXT("Image resolution must be greater than zero.");
			}
			return false;
		}

		if (Fx <= 0.0 || Fy <= 0.0)
		{
			if (OutError)
			{
				*OutError = TEXT("Camera focal length fx/fy must be greater than zero.");
			}
			return false;
		}

		if (Cx < 0.0 || Cx > static_cast<double>(ImageWidth) || Cy < 0.0 || Cy > static_cast<double>(ImageHeight))
		{
			if (OutError)
			{
				*OutError = TEXT("Principal point cx/cy is out of image bounds.");
			}
			return false;
		}

		if (Distortion.Num() > 16)
		{
			if (OutError)
			{
				*OutError = TEXT("Unexpected distortion parameter count.");
			}
			return false;
		}

		return true;
	}

	FString ToSummaryString() const
	{
		return FString::Printf(
			TEXT("%dx%d | fx=%.3f fy=%.3f cx=%.3f cy=%.3f | dist=%d"),
			ImageWidth,
			ImageHeight,
			Fx,
			Fy,
			Cx,
			Cy,
			Distortion.Num());
	}
};

struct FVSCalibratorFrameData
{
	FString SourcePath;
	int32 Width = 0;
	int32 Height = 0;
	FDateTime CaptureTimeUtc = FDateTime::UtcNow();

	bool IsValid() const
	{
		return !SourcePath.IsEmpty() && Width > 0 && Height > 0;
	}

	FString ToSummaryString() const
	{
		return FString::Printf(
			TEXT("%dx%d | %s"),
			Width,
			Height,
			*FPaths::GetCleanFilename(SourcePath));
	}
};

struct FVSCalibratorDepthMap
{
	int32 Width = 0;
	int32 Height = 0;
	TArray<float> Values;
	float MinDepth = 0.0f;
	float MaxDepth = 0.0f;
	float MeanDepth = 0.0f;

	bool IsValid() const
	{
		return Width > 0 && Height > 0 && Values.Num() == Width * Height;
	}

	void RecomputeStats()
	{
		if (Values.IsEmpty())
		{
			MinDepth = 0.0f;
			MaxDepth = 0.0f;
			MeanDepth = 0.0f;
			return;
		}

		float Sum = 0.0f;
		MinDepth = Values[0];
		MaxDepth = Values[0];
		for (const float Value : Values)
		{
			MinDepth = FMath::Min(MinDepth, Value);
			MaxDepth = FMath::Max(MaxDepth, Value);
			Sum += Value;
		}

		MeanDepth = Sum / static_cast<float>(Values.Num());
	}

	FString ToSummaryString() const
	{
		return FString::Printf(
			TEXT("%dx%d | min=%.6f max=%.6f mean=%.6f samples=%d"),
			Width,
			Height,
			MinDepth,
			MaxDepth,
			MeanDepth,
			Values.Num());
	}
};

struct FVSCalibratorPlaneCandidate
{
	FVector Normal = FVector::UpVector;
	float PlaneOffset = 0.0f;
	int32 InlierCount = 0;
	float Confidence = 0.0f;

	FString ToSummaryString() const
	{
		return FString::Printf(
			TEXT("normal=(%.3f,%.3f,%.3f) d=%.4f inliers=%d conf=%.3f"),
			Normal.X,
			Normal.Y,
			Normal.Z,
			PlaneOffset,
			InlierCount,
			Confidence);
	}
};

struct FVSCalibratorManhattanAxes
{
	bool bValid = false;
	FVector AxisX = FVector::ForwardVector;
	FVector AxisY = FVector::RightVector;
	FVector AxisZ = FVector::UpVector;
	float MaxSnapAngleDeg = 8.0f;

	FString ToSummaryString() const
	{
		return FString::Printf(
			TEXT("valid=%s maxSnap=%.1fdeg X=(%.3f,%.3f,%.3f) Y=(%.3f,%.3f,%.3f) Z=(%.3f,%.3f,%.3f)"),
			bValid ? TEXT("true") : TEXT("false"),
			MaxSnapAngleDeg,
			AxisX.X,
			AxisX.Y,
			AxisX.Z,
			AxisY.X,
			AxisY.Y,
			AxisY.Z,
			AxisZ.X,
			AxisZ.Y,
			AxisZ.Z);
	}
};

struct FVSCalibratorGeometryPostprocessConfig
{
	bool bEnablePlaneDetection = false;
	bool bEnableManhattanConstraint = false;
	bool bEnablePlaneProjection = true;
	float PlaneRansacDistanceThreshold = 0.05f;
	float PlaneMinInlierRatio = 0.12f;
	float MaxSnapAngleDeg = 8.0f;
	float ProjectionInlierDistanceThreshold = 0.08f;

	FString ToSummaryString() const
	{
		return FString::Printf(
			TEXT("planeDetect=%s manhattan=%s projection=%s dist=%.3fm minInlier=%.2f maxSnap=%.1fdeg projDist=%.3fm"),
			bEnablePlaneDetection ? TEXT("true") : TEXT("false"),
			bEnableManhattanConstraint ? TEXT("true") : TEXT("false"),
			bEnablePlaneProjection ? TEXT("true") : TEXT("false"),
			PlaneRansacDistanceThreshold,
			PlaneMinInlierRatio,
			MaxSnapAngleDeg,
			ProjectionInlierDistanceThreshold);
	}
};

struct FVSCalibratorGeometryPostprocessResult
{
	TArray<FVector> CameraPoints;
	TArray<FVSCalibratorPlaneCandidate> Planes;
	FVSCalibratorManhattanAxes ManhattanAxes;
	bool bUsedPassthrough = true;
	int32 ProjectedPointCount = 0;
	float MeanAbsPlaneDistBefore = 0.0f;
	float MeanAbsPlaneDistAfter = 0.0f;
	FString StageNote;

	FString ToSummaryString() const
	{
		return FString::Printf(
			TEXT("cameraPts=%d planes=%d projected=%d meanDist=%.4f->%.4f passthrough=%s note=%s"),
			CameraPoints.Num(),
			Planes.Num(),
			ProjectedPointCount,
			MeanAbsPlaneDistBefore,
			MeanAbsPlaneDistAfter,
			bUsedPassthrough ? TEXT("true") : TEXT("false"),
			*StageNote);
	}
};
