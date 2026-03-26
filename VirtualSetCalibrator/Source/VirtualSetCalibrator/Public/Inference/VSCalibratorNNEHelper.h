// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/VSCalibratorTypes.h"

struct FVSCalibratorNNEDepthInferenceResult
{
	bool bRuntimeResolved = false;
	bool bModelCreated = false;
	bool bModelInstanceCreated = false;
	bool bInputShapeConfigured = false;
	bool bRunSyncSucceeded = false;
	bool bPassed = false;
	FString RuntimeName;
	FString InputShape;
	FString OutputShape;
	double InferenceMs = 0.0;
	FVSCalibratorDepthMap DepthMap;
	FString Details;

	FString ToSummaryString() const
	{
		return FString::Printf(
			TEXT("pass=%s runtime=%s runMs=%.2f input=%s output=%s details=%s depth={%s}"),
			bPassed ? TEXT("true") : TEXT("false"),
			RuntimeName.IsEmpty() ? TEXT("<none>") : *RuntimeName,
			InferenceMs,
			InputShape.IsEmpty() ? TEXT("<unknown>") : *InputShape,
			OutputShape.IsEmpty() ? TEXT("<unknown>") : *OutputShape,
			*Details,
			*DepthMap.ToSummaryString());
	}
};

class FVSCalibratorNNEHelper
{
public:
	static FVSCalibratorNNEDepthInferenceResult RunDepthInference(
		const FString& OnnxFilePath,
		const FVSCalibratorFrameData& FrameData,
		double FocalPx);
};
