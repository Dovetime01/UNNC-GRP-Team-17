#include "Inference/VSCalibratorNNEModelUtils.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "NNE.h"
#include "Serialization/MemoryWriter.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	static const FString OnnxExternalDataDescriptorKey(TEXT("OnnxExternalDataDescriptor"));
	static const FString OnnxExternalDataBytesKey(TEXT("OnnxExternalDataBytes"));

	bool ResolveRuntimeCpu(
		FString& OutRuntimeName,
		TWeakInterfacePtr<INNERuntimeCPU>& OutRuntimeCpu,
		FString& OutError)
	{
		OutRuntimeName.Reset();
		OutRuntimeCpu = nullptr;

		const TArray<FString> RuntimeCandidates =
		{
			TEXT("NNERuntimeORTCpu"),
			TEXT("NNERuntimeBasicCpu")
		};

		for (const FString& Candidate : RuntimeCandidates)
		{
			OutRuntimeCpu = UE::NNE::GetRuntime<INNERuntimeCPU>(Candidate);
			if (OutRuntimeCpu.IsValid())
			{
				OutRuntimeName = Candidate;
				return true;
			}
		}

		OutError = TEXT("No CPU runtime was found. Expected NNERuntimeORTCpu or NNERuntimeBasicCpu.");
		return false;
	}

	bool BuildAdditionalOnnxBuffers(
		const FString& OnnxFilePath,
		TMap<FString, TConstArrayView64<uint8>>& OutAdditionalBuffers,
		TArray64<uint8>& OutDescriptorBuffer,
		TArray64<uint8>& OutMergedExternalBytes,
		FString& OutError)
	{
		OutAdditionalBuffers.Reset();
		OutDescriptorBuffer.Reset();
		OutMergedExternalBytes.Reset();

		const FString Directory = FPaths::GetPath(OnnxFilePath);
		TArray<FString> OnnxDataFileNames;
		IFileManager::Get().FindFiles(OnnxDataFileNames, *(Directory / TEXT("*.onnx_data")), true, false);

		if (OnnxDataFileNames.IsEmpty())
		{
			return true;
		}

		OnnxDataFileNames.Sort();

		TMap<FString, int64> ExternalDataDescriptor;
		for (const FString& DataFileName : OnnxDataFileNames)
		{
			const FString DataPath = FPaths::Combine(Directory, DataFileName);
			TArray64<uint8> DataBytes;
			if (!FFileHelper::LoadFileToArray(DataBytes, *DataPath))
			{
				OutError = FString::Printf(TEXT("Failed to read .onnx_data file: %s"), *DataPath);
				return false;
			}

			ExternalDataDescriptor.Add(DataFileName, DataBytes.Num());
			if (DataBytes.Num() > 0)
			{
				const int64 Offset = OutMergedExternalBytes.Num();
				OutMergedExternalBytes.AddUninitialized(DataBytes.Num());
				FMemory::Memcpy(OutMergedExternalBytes.GetData() + Offset, DataBytes.GetData(), DataBytes.Num());
			}
		}

		FMemoryWriter64 DescriptorWriter(OutDescriptorBuffer, true);
		DescriptorWriter << ExternalDataDescriptor;

		OutAdditionalBuffers.Add(OnnxExternalDataDescriptorKey, TConstArrayView64<uint8>(OutDescriptorBuffer));
		OutAdditionalBuffers.Add(OnnxExternalDataBytesKey, TConstArrayView64<uint8>(OutMergedExternalBytes));
		return true;
	}
}

bool VSCalibratorNNEPrivate::CreateModelResources(
	const FString& OnnxFilePath,
	FVSCalibratorNNEModelResources& OutResources,
	FString& OutError)
{
	OutResources = FVSCalibratorNNEModelResources();
	OutError.Reset();

	if (OnnxFilePath.IsEmpty() || !FPaths::FileExists(OnnxFilePath))
	{
		OutError = TEXT("The .onnx model file does not exist.");
		return false;
	}

	TWeakInterfacePtr<INNERuntimeCPU> RuntimeCpu;
	if (!ResolveRuntimeCpu(OutResources.RuntimeName, RuntimeCpu, OutError))
	{
		return false;
	}
	OutResources.bRuntimeResolved = true;

	if (!FFileHelper::LoadFileToArray(OutResources.OnnxBytes, *OnnxFilePath))
	{
		OutError = TEXT("Failed to read the .onnx model file.");
		return false;
	}

	if (OutResources.OnnxBytes.IsEmpty())
	{
		OutError = TEXT("The .onnx model file is empty.");
		return false;
	}

	if (!BuildAdditionalOnnxBuffers(
		OnnxFilePath,
		OutResources.AdditionalBuffers,
		OutResources.DescriptorBuffer,
		OutResources.MergedExternalBytes,
		OutError))
	{
		return false;
	}

	OutResources.ModelData.Reset(NewObject<UNNEModelData>(GetTransientPackage(), NAME_None, RF_Transient));
	if (OutResources.ModelData.Get() == nullptr)
	{
		OutError = TEXT("Failed to create UNNEModelData.");
		return false;
	}

	OutResources.ModelData->Init(TEXT("onnx"), OutResources.OnnxBytes, OutResources.AdditionalBuffers);

	const INNERuntimeCPU::ECanCreateModelCPUStatus CanCreateStatus = RuntimeCpu->CanCreateModelCPU(OutResources.ModelData.Get());
	if (CanCreateStatus != INNERuntimeCPU::ECanCreateModelCPUStatus::Ok)
	{
		OutError = TEXT("The selected runtime rejected CreateModelCPU for this ONNX model.");
		return false;
	}

	OutResources.ModelCpu = RuntimeCpu->CreateModelCPU(OutResources.ModelData.Get());
	OutResources.bModelCreated = OutResources.ModelCpu.IsValid();
	if (!OutResources.bModelCreated)
	{
		OutError = TEXT("CreateModelCPU returned a null model.");
		return false;
	}

	OutResources.ModelInstance = OutResources.ModelCpu->CreateModelInstanceCPU();
	OutResources.bModelInstanceCreated = OutResources.ModelInstance.IsValid();
	if (!OutResources.bModelInstanceCreated)
	{
		OutError = TEXT("CreateModelInstanceCPU returned a null model instance.");
		return false;
	}

	return true;
}
