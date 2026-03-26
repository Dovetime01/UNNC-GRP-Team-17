// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inference/VSCalibratorNNEHelper.h"

#include "HAL/PlatformTime.h"
#include "Inference/VSCalibratorNNEModelUtils.h"
#include "Inference/VSCalibratorNNETensorUtils.h"
#include "NNE.h"

namespace
{
	int32 SelectDepthOutputIndex(
		const TArray<UE::NNE::FTensorShape>& ResolvedOutputShapes,
		const TArray<TArray<float>>& OutputBuffers)
	{
		int32 SelectedOutputIndex = 0;
		int32 BestScore = -1;

		for (int32 OutputIndex = 0; OutputIndex < ResolvedOutputShapes.Num(); ++OutputIndex)
		{
			int32 CandidateW = 0;
			int32 CandidateH = 0;
			VSCalibratorNNEPrivate::ResolveDepthLayout(ResolvedOutputShapes[OutputIndex], CandidateW, CandidateH);

			const int32 CandidateCount = OutputBuffers[OutputIndex].Num();
			const bool bMatches2D = CandidateW > 0 && CandidateH > 0 && CandidateW * CandidateH == CandidateCount;

			int32 Score = bMatches2D ? 3 : 1;
			if (ResolvedOutputShapes[OutputIndex].Rank() <= 4)
			{
				Score += 1;
			}

			if (Score > BestScore)
			{
				BestScore = Score;
				SelectedOutputIndex = OutputIndex;
			}
		}

		return SelectedOutputIndex;
	}
}

FVSCalibratorNNEDepthInferenceResult FVSCalibratorNNEHelper::RunDepthInference(
	const FString& OnnxFilePath,
	const FVSCalibratorFrameData& FrameData,
	double FocalPx)
{
	FVSCalibratorNNEDepthInferenceResult Result;

	if (!FrameData.IsValid())
	{
		Result.Details = TEXT("The input frame is invalid.");
		return Result;
	}

	VSCalibratorNNEPrivate::FVSCalibratorNNEModelResources ModelResources;
	FString ModelError;
	if (!VSCalibratorNNEPrivate::CreateModelResources(OnnxFilePath, ModelResources, ModelError))
	{
		Result.bRuntimeResolved = ModelResources.bRuntimeResolved;
		Result.bModelCreated = ModelResources.bModelCreated;
		Result.bModelInstanceCreated = ModelResources.bModelInstanceCreated;
		Result.RuntimeName = ModelResources.RuntimeName;
		Result.Details = ModelError;
		return Result;
	}

	Result.bRuntimeResolved = ModelResources.bRuntimeResolved;
	Result.bModelCreated = ModelResources.bModelCreated;
	Result.bModelInstanceCreated = ModelResources.bModelInstanceCreated;
	Result.RuntimeName = ModelResources.RuntimeName;

	const TSharedPtr<UE::NNE::IModelInstanceCPU> ModelInstance = ModelResources.ModelInstance;
	const TConstArrayView<UE::NNE::FTensorDesc> InputDescs = ModelInstance->GetInputTensorDescs();
	const TConstArrayView<UE::NNE::FTensorDesc> OutputDescs = ModelInstance->GetOutputTensorDescs();

	if (InputDescs.Num() < 1 || InputDescs.Num() > 2 || OutputDescs.Num() < 1)
	{
		Result.Details = TEXT("Only 1 or 2 inputs are supported (pixel_values plus optional focal_px), and at least one output is required.");
		return Result;
	}

	for (const UE::NNE::FTensorDesc& InputDesc : InputDescs)
	{
		if (InputDesc.GetDataType() != ENNETensorDataType::Float)
		{
			Result.Details = TEXT("Only Float input tensors are supported.");
			return Result;
		}
	}

	for (const UE::NNE::FTensorDesc& OutputDesc : OutputDescs)
	{
		if (OutputDesc.GetDataType() != ENNETensorDataType::Float)
		{
			Result.Details = TEXT("Only Float output tensors are supported.");
			return Result;
		}
	}

	int32 PixelInputIndex = INDEX_NONE;
	for (int32 InputIndex = 0; InputIndex < InputDescs.Num(); ++InputIndex)
	{
		if (VSCalibratorNNEPrivate::IsImageTensorDesc(InputDescs[InputIndex]))
		{
			PixelInputIndex = InputIndex;
			break;
		}
	}

	if (PixelInputIndex == INDEX_NONE)
	{
		Result.Details = TEXT("No recognized image input tensor (pixel_values) was found.");
		return Result;
	}

	int32 StableTargetW = 0;
	int32 StableTargetH = 0;
	VSCalibratorNNEPrivate::ResolveStableImageTargetSize(InputDescs[PixelInputIndex], StableTargetW, StableTargetH);

	TArray<uint8> DecodedBGRA;
	int32 DecodedW = 0;
	int32 DecodedH = 0;
	FString DecodeError;
	if (!VSCalibratorNNEPrivate::DecodeImageToBGRA(FrameData.SourcePath, DecodedBGRA, DecodedW, DecodedH, DecodeError))
	{
		Result.Details = DecodeError;
		return Result;
	}

	TArray<UE::NNE::FTensorShape> InputShapes;
	InputShapes.SetNum(InputDescs.Num());

	bool bInputSymbolicDynamic = false;
	for (int32 InputIndex = 0; InputIndex < InputDescs.Num(); ++InputIndex)
	{
		InputShapes[InputIndex] =
			VSCalibratorNNEPrivate::BuildConcreteInputShape(InputDescs[InputIndex], StableTargetW, StableTargetH);
		bInputSymbolicDynamic = bInputSymbolicDynamic || !InputDescs[InputIndex].GetShape().IsConcrete();
	}

	int32 FocalInputIndex = INDEX_NONE;
	if (InputShapes.Num() == 2)
	{
		FocalInputIndex = (PixelInputIndex == 0) ? 1 : 0;
		if (!VSCalibratorNNEPrivate::IsScalarLikeShape(InputShapes[FocalInputIndex]))
		{
			Result.Details = TEXT("The focal_px input shape must be scalar-like (scalar, [1], or all-one dimensions).");
			return Result;
		}
	}

	bool bOutputSymbolicDynamic = false;
	for (const UE::NNE::FTensorDesc& OutputDesc : OutputDescs)
	{
		bOutputSymbolicDynamic = bOutputSymbolicDynamic || !OutputDesc.GetShape().IsConcrete();
	}

	Result.InputShape = FString::Printf(
		TEXT("resolved=%s symbolic=%s pixelInput=%d focalInput=%d focalPx=%.3f"),
		*VSCalibratorNNEPrivate::TensorShapeListToString(InputShapes),
		*VSCalibratorNNEPrivate::SymbolicTensorShapeListToString(InputDescs),
		PixelInputIndex,
		FocalInputIndex,
		FocalPx);

	if (ModelInstance->SetInputTensorShapes(InputShapes) != UE::NNE::IModelInstanceCPU::ESetInputTensorShapesStatus::Ok)
	{
		Result.Details = FString::Printf(
			TEXT("SetInputTensorShapes failed. inputDynamic=%s input=%s"),
			bInputSymbolicDynamic ? TEXT("true") : TEXT("false"),
			*Result.InputShape);
		return Result;
	}
	Result.bInputShapeConfigured = true;

	const TConstArrayView<UE::NNE::FTensorShape> PreRunOutputShapes = ModelInstance->GetOutputTensorShapes();
	TArray<UE::NNE::FTensorShape> ResolvedOutputShapes;
	ResolvedOutputShapes.SetNum(OutputDescs.Num());

	bool bUsedFallbackOutputShape = false;
	for (int32 OutputIndex = 0; OutputIndex < OutputDescs.Num(); ++OutputIndex)
	{
		if (PreRunOutputShapes.IsValidIndex(OutputIndex))
		{
			ResolvedOutputShapes[OutputIndex] = PreRunOutputShapes[OutputIndex];
		}
		else
		{
			ResolvedOutputShapes[OutputIndex] =
				VSCalibratorNNEPrivate::BuildFallbackOutputShape(OutputDescs[OutputIndex], InputShapes[PixelInputIndex]);
			bUsedFallbackOutputShape = true;
		}
	}

	Result.OutputShape = FString::Printf(
		TEXT("resolved=%s symbolic=%s fallback=%s preCount=%d"),
		*VSCalibratorNNEPrivate::TensorShapeListToString(ResolvedOutputShapes),
		*VSCalibratorNNEPrivate::SymbolicTensorShapeListToString(OutputDescs),
		bUsedFallbackOutputShape ? TEXT("true") : TEXT("false"),
		PreRunOutputShapes.Num());

	TArray<TArray<float>> InputBuffers;
	InputBuffers.SetNum(InputDescs.Num());

	float ImageScaleToModel = 1.0f;
	int32 PadX = 0;
	int32 PadY = 0;

	FString FillInputError;
	if (!VSCalibratorNNEPrivate::FillInputTensorFromBGRA(
		DecodedBGRA,
		DecodedW,
		DecodedH,
		InputShapes[PixelInputIndex],
		InputBuffers[PixelInputIndex],
		ImageScaleToModel,
		PadX,
		PadY,
		FillInputError))
	{
		Result.Details = FillInputError;
		return Result;
	}

	const float MappedFocalPx = static_cast<float>(FocalPx) * ImageScaleToModel;
	if (FocalInputIndex != INDEX_NONE)
	{
		int32 FocalElementCount = static_cast<int32>(InputShapes[FocalInputIndex].Volume());
		if (FocalElementCount <= 0)
		{
			FocalElementCount = 1;
		}

		InputBuffers[FocalInputIndex].SetNumUninitialized(FocalElementCount);
		for (int32 ElementIndex = 0; ElementIndex < FocalElementCount; ++ElementIndex)
		{
			InputBuffers[FocalInputIndex][ElementIndex] = MappedFocalPx;
		}
	}

	TArray<TArray<float>> OutputBuffers;
	OutputBuffers.SetNum(OutputDescs.Num());

	TArray<UE::NNE::FTensorBindingCPU> OutputBindings;
	OutputBindings.Reserve(OutputDescs.Num());
	for (int32 OutputIndex = 0; OutputIndex < OutputDescs.Num(); ++OutputIndex)
	{
		OutputBuffers[OutputIndex].SetNumZeroed(static_cast<int32>(ResolvedOutputShapes[OutputIndex].Volume()));
		OutputBindings.Add(UE::NNE::FTensorBindingCPU{
			OutputBuffers[OutputIndex].GetData(),
			static_cast<uint64>(OutputBuffers[OutputIndex].Num() * sizeof(float))});
	}

	TArray<UE::NNE::FTensorBindingCPU> InputBindings;
	InputBindings.Reserve(InputDescs.Num());
	for (int32 InputIndex = 0; InputIndex < InputDescs.Num(); ++InputIndex)
	{
		if (InputBuffers[InputIndex].IsEmpty())
		{
			Result.Details = FString::Printf(TEXT("Input tensor #%d was not populated."), InputIndex);
			return Result;
		}

		InputBindings.Add(UE::NNE::FTensorBindingCPU{
			InputBuffers[InputIndex].GetData(),
			static_cast<uint64>(InputBuffers[InputIndex].Num() * sizeof(float))});
	}

	const double StartTime = FPlatformTime::Seconds();
	const UE::NNE::IModelInstanceCPU::ERunSyncStatus RunStatus = ModelInstance->RunSync(InputBindings, OutputBindings);
	Result.InferenceMs = (FPlatformTime::Seconds() - StartTime) * 1000.0;

	Result.bRunSyncSucceeded = (RunStatus == UE::NNE::IModelInstanceCPU::ERunSyncStatus::Ok);
	if (!Result.bRunSyncSucceeded)
	{
		Result.Details = FString::Printf(
			TEXT("RunSync failed. fallbackUsed=%s inputDynamic=%s outputDynamic=%s input=%s output=%s preprocess=letterbox scale=%.6f pad=(%d,%d) focal(src/model)=%.3f/%.3f"),
			bUsedFallbackOutputShape ? TEXT("true") : TEXT("false"),
			bInputSymbolicDynamic ? TEXT("true") : TEXT("false"),
			bOutputSymbolicDynamic ? TEXT("true") : TEXT("false"),
			*Result.InputShape,
			*Result.OutputShape,
			ImageScaleToModel,
			PadX,
			PadY,
			FocalPx,
			MappedFocalPx);
		return Result;
	}

	const TConstArrayView<UE::NNE::FTensorShape> PostRunOutputShapes = ModelInstance->GetOutputTensorShapes();
	if (PostRunOutputShapes.Num() > 0)
	{
		for (int32 OutputIndex = 0; OutputIndex < ResolvedOutputShapes.Num(); ++OutputIndex)
		{
			if (PostRunOutputShapes.IsValidIndex(OutputIndex))
			{
				ResolvedOutputShapes[OutputIndex] = PostRunOutputShapes[OutputIndex];
			}
		}

		Result.OutputShape = FString::Printf(
			TEXT("resolved=%s symbolic=%s fallback=%s preCount=%d postCount=%d"),
			*VSCalibratorNNEPrivate::TensorShapeListToString(ResolvedOutputShapes),
			*VSCalibratorNNEPrivate::SymbolicTensorShapeListToString(OutputDescs),
			bUsedFallbackOutputShape ? TEXT("true") : TEXT("false"),
			PreRunOutputShapes.Num(),
			PostRunOutputShapes.Num());
	}

	const int32 SelectedOutputIndex = SelectDepthOutputIndex(ResolvedOutputShapes, OutputBuffers);

	int32 DepthW = 0;
	int32 DepthH = 0;
	VSCalibratorNNEPrivate::ResolveDepthLayout(ResolvedOutputShapes[SelectedOutputIndex], DepthW, DepthH);

	const int32 OutputCount = OutputBuffers[SelectedOutputIndex].Num();
	if (DepthW <= 0 || DepthH <= 0 || DepthW * DepthH != OutputCount)
	{
		DepthW = OutputCount;
		DepthH = 1;
	}

	Result.DepthMap.Width = DepthW;
	Result.DepthMap.Height = DepthH;
	Result.DepthMap.Values = MoveTemp(OutputBuffers[SelectedOutputIndex]);
	Result.DepthMap.RecomputeStats();
	Result.bPassed = Result.DepthMap.IsValid();

	if (!Result.bPassed)
	{
		Result.Details = FString::Printf(
			TEXT("The resolved depth output is invalid. fallbackUsed=%s output=%s"),
			bUsedFallbackOutputShape ? TEXT("true") : TEXT("false"),
			*Result.OutputShape);
		return Result;
	}

	Result.Details = FString::Printf(
		TEXT("RunSync succeeded. fallbackUsed=%s inputDynamic=%s outputDynamic=%s selectedOutput=%d totalOutputs=%d focal(src/model)=%.3f/%.3f preprocess=letterbox scale=%.6f pad=(%d,%d)"),
		bUsedFallbackOutputShape ? TEXT("true") : TEXT("false"),
		bInputSymbolicDynamic ? TEXT("true") : TEXT("false"),
		bOutputSymbolicDynamic ? TEXT("true") : TEXT("false"),
		SelectedOutputIndex,
		OutputDescs.Num(),
		FocalPx,
		MappedFocalPx,
		ImageScaleToModel,
		PadX,
		PadY);
	return Result;
}
