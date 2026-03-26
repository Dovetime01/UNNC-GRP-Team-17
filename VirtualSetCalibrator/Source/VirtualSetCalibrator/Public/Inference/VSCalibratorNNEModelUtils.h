#pragma once

#include "CoreMinimal.h"
#include "NNEModelData.h"
#include "NNERuntimeCPU.h"
#include "UObject/StrongObjectPtr.h"

namespace VSCalibratorNNEPrivate
{
	struct FVSCalibratorNNEModelResources
	{
		bool bRuntimeResolved = false;
		bool bModelCreated = false;
		bool bModelInstanceCreated = false;
		FString RuntimeName;
		TArray64<uint8> OnnxBytes;
		TArray64<uint8> DescriptorBuffer;
		TArray64<uint8> MergedExternalBytes;
		TMap<FString, TConstArrayView64<uint8>> AdditionalBuffers;
		TStrongObjectPtr<UNNEModelData> ModelData;
		TSharedPtr<UE::NNE::IModelCPU> ModelCpu;
		TSharedPtr<UE::NNE::IModelInstanceCPU> ModelInstance;
	};

	bool CreateModelResources(
		const FString& OnnxFilePath,
		FVSCalibratorNNEModelResources& OutResources,
		FString& OutError);
}
