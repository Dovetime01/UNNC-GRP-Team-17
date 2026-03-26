// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/VSCalibratorTypes.h"

namespace VSCalibratorPointCloudUtils
{
	bool BuildPointCloudFromDepth(
		const FVSCalibratorDepthMap& DepthMap,
		const FVSCalibratorCameraIntrinsics& Intrinsics,
		TArray<FVector>& OutPoints,
		FString& OutError);

	bool SavePointCloudAsXyz(const FString& OutputPath, const TArray<FVector>& Points, FString& OutError);
}
