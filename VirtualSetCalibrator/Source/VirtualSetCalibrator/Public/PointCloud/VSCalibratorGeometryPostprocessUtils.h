// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/VSCalibratorTypes.h"

namespace VSCalibratorGeometryPostprocessUtils
{
	bool RunPostDepthGeometry(
		const TArray<FVector>& InCameraPoints,
		const FVSCalibratorGeometryPostprocessConfig& Config,
		FVSCalibratorGeometryPostprocessResult& OutResult,
		FString& OutError);
}
