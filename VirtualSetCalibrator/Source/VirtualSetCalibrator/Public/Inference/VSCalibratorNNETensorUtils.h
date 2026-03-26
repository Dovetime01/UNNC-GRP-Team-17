#pragma once

#include "CoreMinimal.h"
#include "NNETypes.h"

namespace VSCalibratorNNEPrivate
{
	FString TensorShapeListToString(const TArray<UE::NNE::FTensorShape>& Shapes);

	FString SymbolicTensorShapeListToString(TConstArrayView<UE::NNE::FTensorDesc> Descs);

	bool IsImageTensorDesc(const UE::NNE::FTensorDesc& Desc);

	bool IsScalarLikeShape(const UE::NNE::FTensorShape& Shape);

	UE::NNE::FTensorShape BuildConcreteInputShape(
		const UE::NNE::FTensorDesc& Desc,
		int32 ImageTargetWidth,
		int32 ImageTargetHeight);

	void ResolveStableImageTargetSize(const UE::NNE::FTensorDesc& Desc, int32& OutTargetWidth, int32& OutTargetHeight);

	UE::NNE::FTensorShape BuildFallbackOutputShape(
		const UE::NNE::FTensorDesc& OutputDesc,
		const UE::NNE::FTensorShape& InputShape);

	bool DecodeImageToBGRA(
		const FString& FilePath,
		TArray<uint8>& OutBGRA,
		int32& OutWidth,
		int32& OutHeight,
		FString& OutError);

	bool FillInputTensorFromBGRA(
		const TArray<uint8>& Bgra,
		int32 Width,
		int32 Height,
		const UE::NNE::FTensorShape& InputShape,
		TArray<float>& OutInput,
		float& OutImageScale,
		int32& OutPadX,
		int32& OutPadY,
		FString& OutError);

	void ResolveDepthLayout(const UE::NNE::FTensorShape& OutputShape, int32& OutWidth, int32& OutHeight);
}
