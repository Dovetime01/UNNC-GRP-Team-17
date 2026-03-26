// Copyright Epic Games, Inc. All Rights Reserved.

#include "PointCloud/VSCalibratorPointCloudUtils.h"

#include "Misc/FileHelper.h"

namespace VSCalibratorPointCloudUtils
{
	bool BuildPointCloudFromDepth(
		const FVSCalibratorDepthMap& DepthMap,
		const FVSCalibratorCameraIntrinsics& Intrinsics,
		TArray<FVector>& OutPoints,
		FString& OutError)
	{
		OutPoints.Reset();

		if (!DepthMap.IsValid())
		{
			OutError = TEXT("Depth map is invalid.");
			return false;
		}

		FString IntrinsicsError;
		if (!Intrinsics.IsValid(&IntrinsicsError))
		{
			OutError = FString::Printf(TEXT("Invalid camera intrinsics: %s"), *IntrinsicsError);
			return false;
		}

		const float ScaleX = static_cast<float>(DepthMap.Width) / static_cast<float>(Intrinsics.ImageWidth);
		const float ScaleY = static_cast<float>(DepthMap.Height) / static_cast<float>(Intrinsics.ImageHeight);
		const float Fx = static_cast<float>(Intrinsics.Fx) * ScaleX;
		const float Fy = static_cast<float>(Intrinsics.Fy) * ScaleY;
		const float Cx = static_cast<float>(Intrinsics.Cx) * ScaleX;
		const float Cy = static_cast<float>(Intrinsics.Cy) * ScaleY;

		if (Fx <= KINDA_SMALL_NUMBER || Fy <= KINDA_SMALL_NUMBER)
		{
			OutError = TEXT("Scaled focal length is invalid.");
			return false;
		}

		OutPoints.Reserve(DepthMap.Values.Num());
		for (int32 Y = 0; Y < DepthMap.Height; ++Y)
		{
			for (int32 X = 0; X < DepthMap.Width; ++X)
			{
				const int32 Index = Y * DepthMap.Width + X;
				const float Z = DepthMap.Values[Index];
				if (!FMath::IsFinite(Z) || Z <= 0.0f)
				{
					continue;
				}

				const float CameraX = (static_cast<float>(X) - Cx) * Z / Fx;
				const float CameraY = (static_cast<float>(Y) - Cy) * Z / Fy;
				OutPoints.Add(FVector(CameraX, CameraY, Z));
			}
		}

		if (OutPoints.IsEmpty())
		{
			OutError = TEXT("Point cloud is empty after back projection.");
			return false;
		}

		return true;
	}

	bool SavePointCloudAsXyz(const FString& OutputPath, const TArray<FVector>& Points, FString& OutError)
	{
		FString Xyz;
		Xyz.Reserve(Points.Num() * 36);
		for (const FVector& Point : Points)
		{
			Xyz += FString::Printf(TEXT("%.6f %.6f %.6f\n"), Point.X, Point.Y, Point.Z);
		}

		if (!FFileHelper::SaveStringToFile(Xyz, *OutputPath))
		{
			OutError = TEXT("Failed to write XYZ file.");
			return false;
		}

		return true;
	}
}
