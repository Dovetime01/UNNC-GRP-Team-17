#include "Inference/VSCalibratorNNETensorUtils.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"

namespace
{
	static constexpr int32 DefaultStableInputSize = 504;

	FString TensorShapeToString(const UE::NNE::FTensorShape& Shape)
	{
		FString Out(TEXT("["));
		const TConstArrayView<uint32> Data = Shape.GetData();
		for (int32 Index = 0; Index < Data.Num(); ++Index)
		{
			if (Index > 0)
			{
				Out += TEXT(",");
			}
			Out += FString::FromInt(static_cast<int32>(Data[Index]));
		}
		Out += TEXT("]");
		return Out;
	}

	FString SymbolicTensorShapeToString(const UE::NNE::FSymbolicTensorShape& Shape)
	{
		FString Out(TEXT("["));
		const TConstArrayView<int32> Data = Shape.GetData();
		for (int32 Index = 0; Index < Data.Num(); ++Index)
		{
			if (Index > 0)
			{
				Out += TEXT(",");
			}
			Out += FString::FromInt(Data[Index]);
		}
		Out += TEXT("]");
		return Out;
	}

	void ResizeBGRA8LetterboxNearest(
		const TArray<uint8>& Src,
		int32 SrcW,
		int32 SrcH,
		int32 DstW,
		int32 DstH,
		TArray<uint8>& Dst,
		float& OutScale,
		int32& OutPadX,
		int32& OutPadY)
	{
		Dst.SetNumZeroed(DstW * DstH * 4);
		for (int32 PixelIndex = 0; PixelIndex < DstW * DstH; ++PixelIndex)
		{
			Dst[PixelIndex * 4 + 3] = 255;
		}

		const float ScaleX = static_cast<float>(DstW) / static_cast<float>(FMath::Max(1, SrcW));
		const float ScaleY = static_cast<float>(DstH) / static_cast<float>(FMath::Max(1, SrcH));
		OutScale = FMath::Min(ScaleX, ScaleY);

		const int32 ScaledW = FMath::Clamp(FMath::RoundToInt(static_cast<float>(SrcW) * OutScale), 1, DstW);
		const int32 ScaledH = FMath::Clamp(FMath::RoundToInt(static_cast<float>(SrcH) * OutScale), 1, DstH);
		OutPadX = (DstW - ScaledW) / 2;
		OutPadY = (DstH - ScaledH) / 2;

		for (int32 Y = 0; Y < ScaledH; ++Y)
		{
			const int32 SrcY = FMath::Clamp((Y * SrcH) / ScaledH, 0, SrcH - 1);
			for (int32 X = 0; X < ScaledW; ++X)
			{
				const int32 SrcX = FMath::Clamp((X * SrcW) / ScaledW, 0, SrcW - 1);
				const int32 SrcIndex = (SrcY * SrcW + SrcX) * 4;
				const int32 DstIndex = ((Y + OutPadY) * DstW + (X + OutPadX)) * 4;
				Dst[DstIndex + 0] = Src[SrcIndex + 0];
				Dst[DstIndex + 1] = Src[SrcIndex + 1];
				Dst[DstIndex + 2] = Src[SrcIndex + 2];
				Dst[DstIndex + 3] = Src[SrcIndex + 3];
			}
		}
	}
}

FString VSCalibratorNNEPrivate::TensorShapeListToString(const TArray<UE::NNE::FTensorShape>& Shapes)
{
	FString Out(TEXT("{"));
	for (int32 Index = 0; Index < Shapes.Num(); ++Index)
	{
		if (Index > 0)
		{
			Out += TEXT(", ");
		}
		Out += FString::Printf(TEXT("#%d=%s"), Index, *TensorShapeToString(Shapes[Index]));
	}
	Out += TEXT("}");
	return Out;
}

FString VSCalibratorNNEPrivate::SymbolicTensorShapeListToString(TConstArrayView<UE::NNE::FTensorDesc> Descs)
{
	FString Out(TEXT("{"));
	for (int32 Index = 0; Index < Descs.Num(); ++Index)
	{
		if (Index > 0)
		{
			Out += TEXT(", ");
		}
		Out += FString::Printf(TEXT("#%d=%s"), Index, *SymbolicTensorShapeToString(Descs[Index].GetShape()));
	}
	Out += TEXT("}");
	return Out;
}

bool VSCalibratorNNEPrivate::IsImageTensorDesc(const UE::NNE::FTensorDesc& Desc)
{
	const TConstArrayView<int32> Dims = Desc.GetShape().GetData();
	if (Dims.Num() == 4)
	{
		return Dims[1] == 3 || Dims[3] == 3;
	}
	if (Dims.Num() == 3)
	{
		return Dims[0] == 3 || Dims[2] == 3;
	}
	return false;
}

bool VSCalibratorNNEPrivate::IsScalarLikeShape(const UE::NNE::FTensorShape& Shape)
{
	const TConstArrayView<uint32> Dims = Shape.GetData();
	if (Dims.Num() == 0)
	{
		return true;
	}

	for (const uint32 Dim : Dims)
	{
		if (Dim > 1)
		{
			return false;
		}
	}

	return true;
}

UE::NNE::FTensorShape VSCalibratorNNEPrivate::BuildConcreteInputShape(
	const UE::NNE::FTensorDesc& Desc,
	int32 ImageTargetWidth,
	int32 ImageTargetHeight)
{
	const TConstArrayView<int32> Symbolic = Desc.GetShape().GetData();
	if (Symbolic.Num() == 0)
	{
		return UE::NNE::FTensorShape::Make({1u});
	}

	const bool bIsImage = IsImageTensorDesc(Desc);
	const bool bRank4 = Symbolic.Num() == 4;
	const bool bRank3 = Symbolic.Num() == 3;
	const bool bNchw = bRank4 && Symbolic[1] == 3;
	const bool bNhwc = bRank4 && Symbolic[3] == 3;
	const bool bChw = bRank3 && Symbolic[0] == 3;
	const bool bHwc = bRank3 && Symbolic[2] == 3;

	TArray<uint32> Dims;
	Dims.Reserve(Symbolic.Num());
	for (int32 Index = 0; Index < Symbolic.Num(); ++Index)
	{
		const int32 Symbol = Symbolic[Index];
		if (Symbol > 0)
		{
			Dims.Add(static_cast<uint32>(Symbol));
			continue;
		}

		if (!bIsImage)
		{
			Dims.Add(1u);
			continue;
		}

		if (bNchw)
		{
			if (Index == 0)
			{
				Dims.Add(1u);
			}
			else if (Index == 2)
			{
				Dims.Add(static_cast<uint32>(FMath::Max(1, ImageTargetHeight)));
			}
			else if (Index == 3)
			{
				Dims.Add(static_cast<uint32>(FMath::Max(1, ImageTargetWidth)));
			}
			else
			{
				Dims.Add(1u);
			}
		}
		else if (bNhwc)
		{
			if (Index == 0)
			{
				Dims.Add(1u);
			}
			else if (Index == 1)
			{
				Dims.Add(static_cast<uint32>(FMath::Max(1, ImageTargetHeight)));
			}
			else if (Index == 2)
			{
				Dims.Add(static_cast<uint32>(FMath::Max(1, ImageTargetWidth)));
			}
			else
			{
				Dims.Add(1u);
			}
		}
		else if (bChw)
		{
			if (Index == 1)
			{
				Dims.Add(static_cast<uint32>(FMath::Max(1, ImageTargetHeight)));
			}
			else if (Index == 2)
			{
				Dims.Add(static_cast<uint32>(FMath::Max(1, ImageTargetWidth)));
			}
			else
			{
				Dims.Add(1u);
			}
		}
		else if (bHwc)
		{
			if (Index == 0)
			{
				Dims.Add(static_cast<uint32>(FMath::Max(1, ImageTargetHeight)));
			}
			else if (Index == 1)
			{
				Dims.Add(static_cast<uint32>(FMath::Max(1, ImageTargetWidth)));
			}
			else
			{
				Dims.Add(1u);
			}
		}
		else
		{
			Dims.Add(1u);
		}
	}

	if (Dims.Num() == 0)
	{
		Dims.Add(1u);
	}

	return UE::NNE::FTensorShape::Make(Dims);
}

void VSCalibratorNNEPrivate::ResolveStableImageTargetSize(
	const UE::NNE::FTensorDesc& Desc,
	int32& OutTargetWidth,
	int32& OutTargetHeight)
{
	OutTargetWidth = DefaultStableInputSize;
	OutTargetHeight = DefaultStableInputSize;

	const TConstArrayView<int32> Symbolic = Desc.GetShape().GetData();
	if (Symbolic.Num() == 4)
	{
		if (Symbolic[1] == 3)
		{
			if (Symbolic[3] > 0)
			{
				OutTargetWidth = Symbolic[3];
			}
			if (Symbolic[2] > 0)
			{
				OutTargetHeight = Symbolic[2];
			}
			return;
		}

		if (Symbolic[3] == 3)
		{
			if (Symbolic[2] > 0)
			{
				OutTargetWidth = Symbolic[2];
			}
			if (Symbolic[1] > 0)
			{
				OutTargetHeight = Symbolic[1];
			}
			return;
		}
	}

	if (Symbolic.Num() == 3)
	{
		if (Symbolic[0] == 3)
		{
			if (Symbolic[2] > 0)
			{
				OutTargetWidth = Symbolic[2];
			}
			if (Symbolic[1] > 0)
			{
				OutTargetHeight = Symbolic[1];
			}
			return;
		}

		if (Symbolic[2] == 3)
		{
			if (Symbolic[1] > 0)
			{
				OutTargetWidth = Symbolic[1];
			}
			if (Symbolic[0] > 0)
			{
				OutTargetHeight = Symbolic[0];
			}
		}
	}
}

UE::NNE::FTensorShape VSCalibratorNNEPrivate::BuildFallbackOutputShape(
	const UE::NNE::FTensorDesc& OutputDesc,
	const UE::NNE::FTensorShape& InputShape)
{
	TArray<uint32> Dims;
	const TConstArrayView<int32> Symbolic = OutputDesc.GetShape().GetData();
	const TConstArrayView<uint32> InputDims = InputShape.GetData();

	for (int32 Index = 0; Index < Symbolic.Num(); ++Index)
	{
		const int32 Symbol = Symbolic[Index];
		if (Symbol > 0)
		{
			Dims.Add(static_cast<uint32>(Symbol));
		}
		else
		{
			Dims.Add(Index < InputDims.Num() ? InputDims[Index] : 1u);
		}
	}

	if (Dims.Num() == 0)
	{
		Dims = {1u, 1u};
	}

	return UE::NNE::FTensorShape::Make(Dims);
}

bool VSCalibratorNNEPrivate::DecodeImageToBGRA(
	const FString& FilePath,
	TArray<uint8>& OutBGRA,
	int32& OutWidth,
	int32& OutHeight,
	FString& OutError)
{
	OutBGRA.Reset();
	OutWidth = 0;
	OutHeight = 0;

	TArray64<uint8> Compressed;
	if (!FFileHelper::LoadFileToArray(Compressed, *FilePath))
	{
		OutError = TEXT("Failed to read the input image.");
		return false;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	const EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(Compressed.GetData(), Compressed.Num());
	if (ImageFormat == EImageFormat::Invalid)
	{
		OutError = TEXT("The input image format could not be recognized.");
		return false;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(Compressed.GetData(), Compressed.Num()))
	{
		OutError = TEXT("Failed to initialize image decoding.");
		return false;
	}

	TArray64<uint8> RawBGRA;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawBGRA))
	{
		OutError = TEXT("Failed to decode the input image to BGRA.");
		return false;
	}

	OutWidth = static_cast<int32>(ImageWrapper->GetWidth());
	OutHeight = static_cast<int32>(ImageWrapper->GetHeight());
	if (OutWidth <= 0 || OutHeight <= 0)
	{
		OutError = TEXT("The input image resolution is invalid.");
		return false;
	}

	OutBGRA.SetNumUninitialized(static_cast<int32>(RawBGRA.Num()));
	FMemory::Memcpy(OutBGRA.GetData(), RawBGRA.GetData(), RawBGRA.Num());
	return true;
}

bool VSCalibratorNNEPrivate::FillInputTensorFromBGRA(
	const TArray<uint8>& Bgra,
	int32 Width,
	int32 Height,
	const UE::NNE::FTensorShape& InputShape,
	TArray<float>& OutInput,
	float& OutImageScale,
	int32& OutPadX,
	int32& OutPadY,
	FString& OutError)
{
	OutImageScale = 1.0f;
	OutPadX = 0;
	OutPadY = 0;

	const TConstArrayView<uint32> Dims = InputShape.GetData();
	if (Dims.Num() != 3 && Dims.Num() != 4)
	{
		OutError = TEXT("Only rank-3 and rank-4 input tensors are supported.");
		return false;
	}

	int32 TargetH = 0;
	int32 TargetW = 0;
	bool bNchw = false;
	bool bNhwc = false;

	if (Dims.Num() == 4)
	{
		if (Dims[1] == 3)
		{
			bNchw = true;
			TargetH = static_cast<int32>(Dims[2]);
			TargetW = static_cast<int32>(Dims[3]);
		}
		else if (Dims[3] == 3)
		{
			bNhwc = true;
			TargetH = static_cast<int32>(Dims[1]);
			TargetW = static_cast<int32>(Dims[2]);
		}
	}
	else
	{
		if (Dims[0] == 3)
		{
			bNchw = true;
			TargetH = static_cast<int32>(Dims[1]);
			TargetW = static_cast<int32>(Dims[2]);
		}
		else if (Dims[2] == 3)
		{
			bNhwc = true;
			TargetH = static_cast<int32>(Dims[0]);
			TargetW = static_cast<int32>(Dims[1]);
		}
	}

	if (TargetW <= 0 || TargetH <= 0 || (!bNchw && !bNhwc))
	{
		OutError = TEXT("The input tensor shape is not a supported RGB image layout (NCHW or NHWC).");
		return false;
	}

	TArray<uint8> ResizedBGRA;
	if (Width == TargetW && Height == TargetH)
	{
		ResizedBGRA = Bgra;
	}
	else
	{
		ResizeBGRA8LetterboxNearest(Bgra, Width, Height, TargetW, TargetH, ResizedBGRA, OutImageScale, OutPadX, OutPadY);
	}

	OutInput.SetNumZeroed(static_cast<int32>(InputShape.Volume()));

	if (bNchw)
	{
		const int32 PlaneSize = TargetW * TargetH;
		for (int32 Y = 0; Y < TargetH; ++Y)
		{
			for (int32 X = 0; X < TargetW; ++X)
			{
				const int32 PixelIndex = (Y * TargetW + X) * 4;
				const float R = static_cast<float>(ResizedBGRA[PixelIndex + 2]) / 255.0f;
				const float G = static_cast<float>(ResizedBGRA[PixelIndex + 1]) / 255.0f;
				const float B = static_cast<float>(ResizedBGRA[PixelIndex + 0]) / 255.0f;
				const int32 Index = Y * TargetW + X;
				OutInput[Index] = R;
				OutInput[PlaneSize + Index] = G;
				OutInput[PlaneSize * 2 + Index] = B;
			}
		}
	}
	else
	{
		for (int32 Y = 0; Y < TargetH; ++Y)
		{
			for (int32 X = 0; X < TargetW; ++X)
			{
				const int32 PixelIndex = (Y * TargetW + X) * 4;
				const int32 TensorIndex = (Y * TargetW + X) * 3;
				OutInput[TensorIndex + 0] = static_cast<float>(ResizedBGRA[PixelIndex + 2]) / 255.0f;
				OutInput[TensorIndex + 1] = static_cast<float>(ResizedBGRA[PixelIndex + 1]) / 255.0f;
				OutInput[TensorIndex + 2] = static_cast<float>(ResizedBGRA[PixelIndex + 0]) / 255.0f;
			}
		}
	}

	return true;
}

void VSCalibratorNNEPrivate::ResolveDepthLayout(const UE::NNE::FTensorShape& OutputShape, int32& OutWidth, int32& OutHeight)
{
	OutWidth = 0;
	OutHeight = 0;
	const TConstArrayView<uint32> Dims = OutputShape.GetData();

	if (Dims.Num() == 4)
	{
		if (Dims[1] == 1)
		{
			OutHeight = static_cast<int32>(Dims[2]);
			OutWidth = static_cast<int32>(Dims[3]);
			return;
		}
		if (Dims[3] == 1)
		{
			OutHeight = static_cast<int32>(Dims[1]);
			OutWidth = static_cast<int32>(Dims[2]);
			return;
		}
	}

	if (Dims.Num() == 3)
	{
		if (Dims[0] == 1)
		{
			OutHeight = static_cast<int32>(Dims[1]);
			OutWidth = static_cast<int32>(Dims[2]);
			return;
		}
		if (Dims[2] == 1)
		{
			OutHeight = static_cast<int32>(Dims[0]);
			OutWidth = static_cast<int32>(Dims[1]);
			return;
		}
	}

	if (Dims.Num() == 2)
	{
		OutHeight = static_cast<int32>(Dims[0]);
		OutWidth = static_cast<int32>(Dims[1]);
	}
}
